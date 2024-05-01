#version 450 
layout(location = 0) in vec3 vPos;

uniform mat4 _Model; 
uniform mat4 _ViewProjection;
out vec2 screenUV;

uniform float time;
uniform vec2 directions;
uniform float squash;
uniform float intensity;

void main(){
	
	vec4 projected = _ViewProjection * _Model * vec4(vPos,1.0);
	screenUV = (projected.xy/projected.w) * 0.5 + 0.5;

	screenUV += cos(time*directions + screenUV * squash)* intensity;

	gl_Position = projected;
}


//https://www.shadertoy.com/view/4l3SDf <<- Credit for Shader Effect


