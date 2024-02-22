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

uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

uniform sampler2D _HeightMap;
uniform float _Scale;

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal); //Re-normalize normal vector; it may have been distorted when it was interpolated
	vec3 toLight = -_LightDirection;
	vec3 toCamera = normalize(_CameraPos -fs_in.WorldPos);
	
	//Diffuse
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Specular
	vec3 h = normalize(toLight + toCamera); //Blinn-Phong uses half angle
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess);
	
	vec3 lightColor = ((_Material.Kd * diffuseFactor + _Material.Ks * specularFactor) * _LightColor) + _Material.Ka * _AmbientColor;
	vec3 objectColor = vec3(0.0, 0.8, 0.0) + ((texture(_HeightMap, fs_in.TexCoord).rgb - vec3(0.5)) * _Scale/10);
	
	FragColor = vec4(objectColor * lightColor, 1.0);
}