#version 450 core

layout(location=0) in vec3 v_vertex;
layout(location=1) in vec2 v_slime_texcoord;
layout(location=2) in vec3 v_normal;
layout(location=3) in vec4 v_worldPosOffset;
layout(location=4) in vec3 v_foliages_texcoord;

out vec3 f_worldVertex;
out vec3 f_worldNormal;
out vec3 f_viewVertex;
out vec3 f_viewNormal;
out vec3 N;
out vec3 L;
out vec3 V;
out vec2 slime_texcoord;
out vec3 foliages_texcoord;

layout(location = 0) uniform mat4 modelMat ;
layout(location = 1) uniform mat4 viewMat ;
layout(location = 2) uniform mat4 projMat ;
layout(location = 5) uniform int shadingModelId;

uniform vec3 light_pos = vec3(100.0, 100.0, 100.0);

void main(){
	vec4 worldVertex;
	if (shadingModelId == 2) {
		worldVertex = modelMat * vec4(v_vertex, 1.0) + v_worldPosOffset;
	}
	else {
		worldVertex = modelMat * vec4(v_vertex, 1.0);
	}
	vec4 worldNormal = modelMat * vec4(v_normal, 0.0);
	vec4 viewVertex = viewMat * worldVertex;
	vec4 viewNormal = viewMat * worldNormal;
	
	f_worldVertex = worldVertex.xyz;
	f_worldNormal = worldNormal.xyz;
	f_viewVertex = viewVertex.xyz;
	f_viewNormal = viewNormal.xyz;

	N = f_viewNormal;
	L = light_pos - viewVertex.xyz;
	V = -viewVertex.xyz;
	
	slime_texcoord = v_slime_texcoord;
	foliages_texcoord = v_foliages_texcoord;

	gl_Position = projMat * viewVertex;
}