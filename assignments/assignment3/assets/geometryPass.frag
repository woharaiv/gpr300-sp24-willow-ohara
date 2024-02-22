#version 450

layout(location = 0) out vec3 gWorldPos;
layout(location = 1) out vec3 gWorldNormal;
layout(location = 2) out vec3 gAlbedo;

in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
}fs_in;

uniform sampler2D _MainTex;

void main()
{
	gWorldPos = fs_in.WorldPos;
	gWorldNormal = normalize(fs_in.WorldNormal);
	gAlbedo = texture(_MainTex, fs_in.TexCoord).rgb;
}