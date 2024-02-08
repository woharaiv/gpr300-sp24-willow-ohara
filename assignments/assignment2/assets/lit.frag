#version 450

out vec4 FragColor;
in Surface{
	vec3 WorldPos;
	vec4 LightSpacePos;
	vec3 WorldNormal;
	vec2 TexCoord;
	}fs_in;

uniform sampler2D _MainTex;
uniform sampler2D _RoughnessTex;

uniform sampler2D _ShadowMap;

uniform vec3 _CameraPos;


uniform vec3 _LightDirection;
uniform vec3 _LightColor = vec3(1.0); //Pure white

uniform vec3 _AmbientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material _Material;

float ShadowCalc(vec4 lightSpacePos)
{
	//Maps light space position to the range [-1, 1]
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	//Remaps to [0,1]
	projCoords = projCoords * 0.5 + 0.5;
	//When we look towards this fragment from the light's pov, what depth do we see?
	float closestDepth = texture(_ShadowMap, projCoords.xy).r;
	//What is our actual depth?
	float fragmentDepth = projCoords.z;

	float bias = max(0.005 * (1.0 - dot(fs_in.WorldNormal, _LightDirection)), 0.005);
	//Return 0 if our projection is outside of the depth map
	if (projCoords.z > 1.0)
	{ 
		return 0.0;
	}

	//If closestDepth is greater, then something is closer to the light than this fragment, so this fragment is in shadow.
	float inShadow = fragmentDepth - bias > closestDepth ? 1.0 : 0.0;
	return inShadow;
}

void main()
{
	vec3 normal = normalize(fs_in.WorldNormal); //Re-normalize normal vector; it may have been distorted when it was interpolated
	vec3 toLight = -_LightDirection;
	vec3 toCamera = normalize(_CameraPos -fs_in.WorldPos);
	
	//Diffuse
	float diffuseFactor = max(dot(normal, toLight), 0.0);
	float diffuse = diffuseFactor * _Material.Kd;

	//Specular
	vec3 h = normalize(toLight + toCamera); //Blinn-Phong uses half angle
	float specularFactor = pow(max(dot(normal,h),0.0),_Material.Shininess) * (texture(_RoughnessTex,fs_in.TexCoord).r) * 20;
	float specular = specularFactor * _Material.Ks;

	//Shadow
	float shadow = ShadowCalc(fs_in.LightSpacePos);

	vec3 lightColor = ((_Material.Ka *_AmbientColor) + (1.0 - shadow) * (diffuse + specular));
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;
	
	FragColor = vec4(objectColor * lightColor, 1.0);
}