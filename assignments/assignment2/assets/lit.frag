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
uniform float _minShadowBias, _maxShadowBias;

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

float ShadowCalc(vec4 fragPosLightSpace, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(_ShadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, vec3(0, -1, 0))), 0.005);  
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 0.00 : 0.0;        
		}    
	}
	shadow /= 9.0;
    return shadow;
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
	float shadow = ShadowCalc(fs_in.LightSpacePos, normal);
	
	vec3 objectColor = texture(_MainTex, fs_in.TexCoord).rgb;

	FragColor = vec4(objectColor * lightColor, 1.0);
}