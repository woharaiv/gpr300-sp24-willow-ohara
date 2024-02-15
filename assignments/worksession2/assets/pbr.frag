#version 450

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 WorldNormal;

struct PBRMaterial{
	sampler2D albedo;
	sampler2D metallic;
	sampler2D roughness;
	sampler2D occlusion;
	sampler2D specular;
};

uniform PBRMaterial _Material;
uniform vec3 _CameraPos;
uniform vec3 _LightPos;
uniform vec3 _AmbientLight;

const vec3 lightColor = vec3(1.0);
const float PI = 3.14159;
const float EPSILON = 0.000001;

vec3 col;
float mtl;
float rgh;
float spec;
float ao;

vec3 N;
vec3 V;
vec3 L;
vec3 H;

float nDOTh;
float nDOTv;
float nDOTl;
float vDOTh;
float vDOTn;
float lDOTn;
float lDOTv;

//GGX / Throwbridge-Reitz normal distribution function
float D(float alpha)
{
	float denominator = PI * pow((pow(nDOTh, 2.0) * (pow(alpha, 2.0) - 1.0) + 1.0), 2.0);
	return pow(alpha, 2)/max(denominator, EPSILON);
}

//Schlick-Backmann (geometry shading function)
float G1(float alpha, float X)
{
	float k = alpha * 0.5;
	float denominator = max(X * (1.0-k) + k, EPSILON);
	return (X / denominator);
}

//Smith Model
float G(float alpha)
{
	return G1(alpha, lDOTn) * G1(alpha, lDOTv);
}

//Fresnel-Schlick Function
vec3 F(vec3 F0)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0-vDOTh, 5);
}


vec3 PBR()
{
	//Base reflectivity
	vec3 F0 = col;

	//Conservation of energy (objects shouldn't glow)
	vec3 kS = F(F0);
	vec3 kD = (vec3(1.0) - kS) * (1.0 - mtl);

	//BDRF diffuse
	vec3 lambert = col / PI;

	// BDRF specular
	float alpha = pow(rgh, 2.0);

	//Cook-Torrence
	vec3 cookTorrenceEnumerator = D(alpha) * G(alpha) * kS;
	float cookTorrenceDenominator = max(4.0 * vDOTn * lDOTn, EPSILON);
	vec3 cookTorrence = cookTorrenceEnumerator/cookTorrenceDenominator;

	vec3 BDRF = (kD * lambert) + cookTorrence;

	return BDRF * lightColor * lDOTn;
}

void main()
{
	//Pre-sampling
	col = texture(_Material.albedo, TexCoords).rgb;
	mtl = texture(_Material.metallic, TexCoords).r;
	rgh = texture(_Material.roughness, TexCoords).r;
	spec = texture(_Material.specular, TexCoords).r;
	ao = texture(_Material.occlusion, TexCoords).r;

	//Precompute vectors
	vec3 N = normalize(WorldNormal);
	vec3 V = normalize(_CameraPos);
	vec3 L = normalize(_LightPos);
	vec3 H = normalize(V + L);

	//Precompute dot products
	nDOTh = max(dot(N, H), 0.0);
	nDOTv = max(dot(N, V), 0.0);
	nDOTl = max(dot(N, L), 0.0);
	vDOTh = max(dot(V, H), 0.0);
	vDOTn = max(dot(V, N), 0.0);
	lDOTn = max(dot(L, N), 0.0);
	lDOTv = max(dot(L, V), 0.0);

	//Stylization
	vec3 reflectionDir = reflect(-L, N);
	float specAmount = pow(max(dot(V, reflectionDir),0.0),32.0);
	vec3 specular = spec * specAmount * lightColor;

	vec3 finalColor = (_AmbientLight * col * ao) + PBR() + specular;
	FragColor = vec4(finalColor, 1.0);
}