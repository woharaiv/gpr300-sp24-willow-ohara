#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Position in model space

uniform mat4 _Model; //Model -> World matrix 
uniform mat4 _ViewProjection; //View -> Projection matrix 

void main()
{

	//Transform to clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0); 
}