#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Position in model space
layout(location = 1) in vec3 vNormal; //Normal in model space
layout(location = 2) in vec2 vTexCoord; //Texture coordinate

uniform mat4 _Model; //Model -> World matrix 
uniform mat4 _ViewProjection; //View -> Projection matrix 

uniform sampler2D _HeightMap;

uniform float _Scale;

out Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}vs_out;

void main()
{
	float heightMod = texture(_HeightMap, vTexCoord).r * _Scale;
	
	vec3 modelSpacePos = vPos;
	modelSpacePos.y += heightMod;
	//Transform vertex position to world space
	vs_out.WorldPos = vec3(_Model * vec4(modelSpacePos, 1.0));
	//Transform vertex nromal to world space
	vs_out.WorldNormal = transpose(inverse(mat3(_Model))) * vNormal;
	vs_out.TexCoord = vTexCoord;


	//Transform to clip space
	gl_Position = _ViewProjection * _Model * vec4(modelSpacePos, 1.0);
}