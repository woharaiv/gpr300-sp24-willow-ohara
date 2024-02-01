#version 450
//Vertex attributes
layout(location = 0) in vec2 pos; //Position in model space

out vec2 uv;

void main()
{
	uv = pos;
	//Transform to clip space
	gl_Position = vec4(pos.xy * 2.0 - 1.0, 0.5, 1.0); 
}