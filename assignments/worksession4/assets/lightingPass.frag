#version 450

uniform sampler2D gPosition;
uniform sampler2D gNormals;
uniform sampler2D gAlbedo;
uniform sampler2D shadowMap;

uniform mat4 _LightSpaceMatrix;

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 viewPos;

uniform vec3 directionalLightPosition = vec3(20, 2, -20);
uniform vec3 directionalLightColor = vec3(1.0); //Pure white

uniform vec3 ambientColor = vec3(0.3,0.4,0.46);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlight
};
uniform Material material;

struct PointLight {
	vec3 position;
	float radius;
	vec4 color;
};
#define MAX_POINT_LIGHTS 256
uniform PointLight pointLights[MAX_POINT_LIGHTS];

vec3 calculateLighting(vec3 lightPosition, vec3 lightColor, vec3 position, vec3 normal, vec3 albedo)
{
	vec3 normalVec = normalize(normal); //Re-normalize normal vector; it may have been distorted when it was interpolated
	vec3 toLight = normalize(lightPosition - position);
	vec3 toCamera = normalize(viewPos - position);
	
	//Diffuse
	float diffuseFactor = max(dot(normal, toLight), 0.0);

	//Specular
	vec3 h = normalize(toLight + toCamera); //Blinn-Phong uses half angle
	float specularFactor = pow(max(dot(normalVec,h),0.0),material.Shininess);
	
	vec3 totalColor = ((material.Kd * diffuseFactor + material.Ks * specularFactor) * lightColor) + material.Ka * ambientColor;

	return totalColor;
}

float attenuate(float dist, float radius)
{
	float i = clamp(1.0 - pow(dist/radius, 4.0), 0.0, 1.0);
	return i * i;
}

float calcShadow(vec4 fragPosLightSpace, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
	float bias = max(0.05 * (1.0 - dot(normal, vec3(0, -1, 0))), 0.005);  
    float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
			shadow += currentDepth - bias > pcfDepth ? 0.0 : 1.0;        
		}    
	}
	shadow /= 9.0;
    return shadow;
} 

vec3 calcPointLight(PointLight light, vec3 position, vec3 normal, vec3 albedo)
{
	vec3 diff = light.position - position;
	//Normalized direction to light
	vec3 toLight = normalize(diff);
	vec3 lightColor = calculateLighting(light.position, vec3(light.color), position, normal, albedo);
	float d = length(diff);
	lightColor *= attenuate(d, light.radius);
	return lightColor;
}


uniform sampler2D _TransitionGradient;
uniform float _TransitionAmount;
uniform vec3 _TransitionColor = vec3(0.0);

void main()
{
	if(texture(_TransitionGradient, TexCoords).r < _TransitionAmount)
	{
		FragColor = vec4(_TransitionColor, 1.0);
	}
	else
	{
		vec3 position = texture(gPosition, TexCoords).xyz;
		vec3 normal = texture(gNormals, TexCoords).xyz;
		vec3 albedo = texture(gAlbedo, TexCoords).xyz;
		vec3 shadow = texture(shadowMap, TexCoords).xyz;
	
		vec3 light_color = vec3(0);
		light_color += ambientColor;
		light_color += (calculateLighting(directionalLightPosition, directionalLightColor, position, normal, albedo)) * calcShadow(_LightSpaceMatrix * vec4(position, 1.0), normal);
		for(int i = 0; i < MAX_POINT_LIGHTS; i++)
		{
			light_color += calcPointLight(pointLights[i], position, normal, albedo);
		}
		FragColor = vec4(albedo * light_color, 1.0);
	}
}