#version 450

out vec4 FragColor;
in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	}fs_in;

uniform sampler2D _MainTex;

//Light pointing straight down
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //Pure white

void main()
{
	//Re-normalize normal to 1 after it was interpolated
	vec3 normal = normalize(fs_in.WorldNormal);

	vec3 toLight = -_LightDirection;

	float diffuseFactor = max(dot(normal, toLight), 0.0);
	
	vec3 diffuseColor = _LightColor * diffuseFactor;
	
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	
	FragColor = vec4(objectColor * diffuseColor, 1.0);
}