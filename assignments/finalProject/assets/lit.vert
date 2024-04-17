#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Position in model space
layout(location = 1) in vec3 vNormal; //Normal in model space
layout(location = 2) in vec2 vTexCoord; //Texture coordinate

uniform mat4 _Model; //Model -> World matrix 
uniform mat4 _ViewProjection; //View -> Projection matrix 

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

void main()
{
	//Transform vertex position to world space
	vs_out.WorldPos = vec3(_Model * vec4(vPos, 1.0));
	//Transform vertex nromal to world space
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;

	//Transform to clip space
	gl_Position = _ViewProjection * _Model * vec4(vPos, 1.0); 
}