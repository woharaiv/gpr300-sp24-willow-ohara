#version 450

uniform sampler2D gPosition;
uniform sampler2D gNormals;
uniform sampler2D gAlbedo;

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 viewPos;

vec3 directionalLightPosition = vec3(20, 2, -20);
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


void main()
{
	vec3 position = texture(gPosition, TexCoords).xyz;
	vec3 normal = texture(gNormals, TexCoords).xyz;
	vec3 albedo = texture(gAlbedo, TexCoords).xyz;
	
	vec3 light_color = vec3(0);
	light_color += ambientColor;
	for(int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		light_color += calcPointLight(pointLights[i], position, normal, albedo);
	}

	FragColor = vec4(albedo * light_color, 1.0);
}