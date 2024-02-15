#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Position in model space
layout(location = 1) in vec3 vNormal; //Normal in model space
layout(location = 2) in vec2 vTexCoord; //Texture coordinate

uniform mat4 _Model; //Model -> World matrix 
uniform mat4 _ViewProjection; //View -> Projection matrix 

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 WorldNormal;


void main()
{
	//Transform vertex position to world space
	WorldPos = vec3(_Model * vec4(vPos, 1.0));
	//Transform vertex nromal to world space
	WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	TexCoords = vTexCoord;

	//Transform to clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0); 
}