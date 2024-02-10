#version 450 core

in vec3 f_worldVertex;
in vec3 f_worldNormal;
in vec3 f_viewVertex;
in vec3 f_viewNormal;
in vec3 N;
in vec3 L;
in vec3 V;
in vec2 slime_texcoord;
in vec3 foliages_texcoord;

layout (location = 0) out vec4 fragColor ;
layout (location = 5) uniform int shadingModelId;

uniform sampler2D slime_tex;
uniform sampler2DArray foliages_tex;

uniform vec3 diffuse_albedo = vec3(0.5, 0.2, 0.7);
uniform vec3 specular_albedo = vec3(0.1, 0.1, 0.1);
uniform float specular_power = 200.0;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);  

vec4 WithFog(vec4 color){
	const vec4 FOG_COLOR = vec4(0.0, 0.0, 0.0, 1) ;
	const float MAX_DIST = 150.0 ;
	const float MIN_DIST = 120.0 ;
	
	float dis = length(f_viewVertex) ;
	float fogFactor = (MAX_DIST - dis) / (MAX_DIST - MIN_DIST) ;
	fogFactor = clamp(fogFactor, 0.0f, 1.0f) ;
	fogFactor = fogFactor * fogFactor ;
	
	return mix(FOG_COLOR, color, fogFactor) ;
}

float GridFactor(vec3 worldPos){
	const vec2 HALF_LINE_WIDTH = vec2(0.05f, 0.05f);
	// make it repeat
	vec2 fr = fract(worldPos.xz);
	// [0.0, 1.0) -> [-1.0, 1.0)
	fr = 2.0f * fr - 1.0f;
	// fwidth = abs(dFdx) + abs(dFdy)
	vec2 uvDeriv = fwidth(worldPos.xz);
	const vec2 lineAA = uvDeriv * 1.5f;
	// (approximate) constant pixel width
	vec2 finalHalfLineWidth = HALF_LINE_WIDTH * lineAA;
	// prevent from too thickness
	finalHalfLineWidth = max(HALF_LINE_WIDTH, finalHalfLineWidth);
	
	const vec2 SX = vec2(0.0f, 0.0f);	
	const vec2 s = smoothstep(SX - finalHalfLineWidth - lineAA, SX - finalHalfLineWidth,          fr);	
	const vec2 m = smoothstep(SX + finalHalfLineWidth,          SX + finalHalfLineWidth + lineAA, fr);	
	const vec2 res = s - m;
	// fade out the line based on the thickness
	const vec2 fadeOutRes = res * clamp(HALF_LINE_WIDTH / finalHalfLineWidth, vec2(0.0f), vec2(1.0f));	
	// for the area that is usually has Moire patterns, fade out it
	const vec2 deriv2 = clamp(uvDeriv, 0.0f, 1.0f);
	const vec2 finalRes = mix(fadeOutRes, HALF_LINE_WIDTH, deriv2);
	
	return mix(finalRes.x, 1.0, finalRes.y);
}
	
void RenderGrid(){
	const vec4 groundColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	const vec4 gridColor = vec4(0.7f, 0.7f, 0.7f, 1.0f);
	
	vec4 color = mix(groundColor, gridColor, GridFactor(f_worldVertex));
	fragColor = WithFog(color);
}

void RenderViewFrustum(){
	fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}

void RenderSlime() {
	vec4 texColor = texture(slime_tex, slime_texcoord);
	texColor = WithFog(texColor);
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	vec3 v = normalize(V);
	vec3 r = reflect(-l, n);
	vec3 diffuse = max(dot(n, l), 0.0) * texColor.xyz;
	vec3 specular = pow(max(dot(r, v), 0.0), specular_power) * specular_albedo * texColor.xyz;
	vec3 aaa = ambient * texColor.xyz;

	fragColor = vec4(aaa + diffuse + specular, 1.0);
}

void RenderFoliages() {
	vec4 texColor = texture(foliages_tex, foliages_texcoord);
	if (texColor.a < 0.5) {
		discard;
	}
	texColor = WithFog(texColor);
	vec3 n = normalize(N);
	vec3 l = normalize(L);
	vec3 v = normalize(V);
	vec3 r = reflect(-l, n);
	vec3 diffuse = max(dot(n, l), 0.0) * texColor.xyz;
	vec3 specular = pow(max(dot(r, v), 0.0), specular_power) * specular_albedo * texColor.xyz;
	vec3 aaa = ambient * texColor.xyz;

	fragColor = vec4(aaa + diffuse + specular, 1.0);
}

void main(){	
	if(shadingModelId == 5){
		RenderViewFrustum();
	}
	else if (shadingModelId == 1) {
		RenderSlime();
	}
	else if (shadingModelId == 2) {
		RenderFoliages();
	}
	else{
		RenderGrid();
	}	
}