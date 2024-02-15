#version 450
//Vertex attributes
layout(location = 0) in vec3 vPos; //Position in model space
layout(location = 1) in vec3 vNormal; //Normal in model space
layout(location = 2) in vec2 vTexCoord; //Texture coordinate

uniform mat4 _Model; //Model -> World matrix 
uniform mat4 _ViewProjection; //View -> Projection matrix 

out vec2 TexCoords;

uniform float _Time;
uniform float _WaveStrength;
uniform float _WaveScale;

float calculateSurface(float x, float z)
{
	float y = 0.0;
	y += (sin(x * 0.2 / _WaveScale + _Time) + sin(x * 2 / (_WaveScale * 3) + _Time * 0.02) + sin(x * 5.27 / (_WaveScale * 5) + _Time * 1.61)) / 3.0;
	y += (cos(z * 0.06 / _WaveScale + _Time) + cos(z * 1 / (_WaveScale * 3) + _Time * 0.12) + cos(z * 5.72 / (_WaveScale * 5)  + _Time * 1.16)) / 3.0;
	return y;
}

void main()
{
	vec3 pos = vPos;
	pos.y += calculateSurface(pos.x, pos.z) * _WaveStrength;
	TexCoords = vTexCoord;

	//Transform to clip space
	gl_Position = _ViewProjection * _Model * vec4(pos, 1.0); 
}