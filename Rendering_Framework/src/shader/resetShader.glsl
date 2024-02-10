#version 430 core

layout (local_size_x = 3, local_size_y = 1, local_size_z = 1) in ;

struct DrawElementsIndirectCommand{
	uint count ;
	uint instanceCount ;
	uint firstIndex ;
	uint baseVertex ;
	uint baseInstance ;
};

layout (std430, binding=3) buffer DrawCommandsBlock{
	DrawElementsIndirectCommand commands[] ;
};

void main() {
	const uint idx = gl_LocalInvocationID.x;
	commands[idx].instanceCount = 0 ;
}