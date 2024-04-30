#version 450 
layout(location = 0) in vec3 vPos;

uniform mat4 _Model; 
uniform mat4 _ViewProjection;
out vec2 UV;

void main(){
	vec4 projected = _ViewProjection * _Model * vec4(vPos,1.0);
	UV = projected.xy/projected.w;
	gl_Position = projected;
}
