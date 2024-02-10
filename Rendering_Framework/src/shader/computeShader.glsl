#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in ;

struct DrawElementsIndirectCommand{
	uint count ;
	uint instanceCount ;
	uint firstIndex ;
	uint baseVertex ;
	uint baseInstance ;
};

struct RawInstanceProperties{
	vec4 position ;
	ivec4 passed;
};

struct InstanceProperties{
	vec4 position ;
};

layout (std430, binding=1) buffer InstanceData{
	RawInstanceProperties rawInstanceProps[] ;
};

layout (std430, binding=2) buffer CurrValidInstanceData{
	InstanceProperties currValidInstanceProps[] ;
};

layout (std430, binding=3) buffer DrawCommandsBlock{
	DrawElementsIndirectCommand commands[] ;
};

layout (location = 1) uniform mat4 viewProjMat;
layout (location = 6) uniform int grassNumInstance;
layout (location = 7) uniform int bush01NumInstance;
layout (location = 8) uniform int bush05NumInstance;
layout (location = 9) uniform int totalNumInstance;
layout (location = 10) uniform float slimeX;
layout (location = 11) uniform float slimeY;
layout (location = 12) uniform float slimeZ;

void main(){
	const uint idx = gl_GlobalInvocationID.x;

	if(idx >= totalNumInstance) { 
		return; 
	} 

	if (rawInstanceProps[idx].passed == ivec4(1, 1, 1, 1)) {
		return;
	}
	else {
		if (distance(rawInstanceProps[idx].position.xyz, vec3(slimeX, slimeY, slimeZ)) <= 1) {
			rawInstanceProps[idx].passed = ivec4(1, 1, 1, 1);
			return;
		}
	}

	vec4 clipSpaceV = viewProjMat * vec4(rawInstanceProps[idx].position.xyz, 1.0) ;
	clipSpaceV = clipSpaceV / clipSpaceV.w ;

	bool frustumCulled = (clipSpaceV.x < -1.0) || (clipSpaceV.x > 1.0) || (clipSpaceV.y < -1.0) || (clipSpaceV.y > 1.0) || (clipSpaceV.z < -1.0) || (clipSpaceV.z > 1.0) ;	

	if(frustumCulled == false){
		if (idx < grassNumInstance) {
			const uint UNIQUE_IDX = atomicAdd(commands[0].instanceCount, 1);
			currValidInstanceProps[UNIQUE_IDX].position = rawInstanceProps[idx].position ;
		}
		else if (idx < grassNumInstance + bush01NumInstance) {
			const uint UNIQUE_IDX = atomicAdd(commands[1].instanceCount, 1);
			currValidInstanceProps[grassNumInstance + UNIQUE_IDX].position = rawInstanceProps[idx].position ;
		}
		else if (idx < grassNumInstance + bush01NumInstance + bush05NumInstance) {
			const uint UNIQUE_IDX = atomicAdd(commands[2].instanceCount, 1);
			currValidInstanceProps[grassNumInstance + bush01NumInstance + UNIQUE_IDX].position = rawInstanceProps[idx].position ;
		}
	}
}