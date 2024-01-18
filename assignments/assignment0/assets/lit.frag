#version 450

out vec4 FragColor;
in Surface{
	vec3 WorldPos;
	vec3 WorldNormal;
	vec2 TexCoord;
	}fs_in;

uniform sampler2D _MainTex;

uniform vec3 _CameraPos;

//Light pointing straight down
uniform vec3 _LightDirection = vec3(0.0, -1.0, 0.0);
uniform vec3 _LightColor = vec3(1.0); //Pure white

void main()
{
	//Re-normalize normal to 1 after it was interpolated
	vec3 normal = normalize(fs_in.WorldNormal);
	vec3 toLight = -_LightDirection;
	vec3 toCamera = normalize(_CameraPos -fs_in.WorldPos);
	
	//Diffuse
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Specular
	vec3 h = normalize(toLight + toCamera);
	float specularFactor = pow(max(dot(normal,h),0.0),128);
	
	vec3 lightColor = (diffuseFactor + specularFactor) * _LightColor;
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	
	FragColor = vec4(objectColor * lightColor, 1.0);
}