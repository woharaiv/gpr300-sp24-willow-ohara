#version 450

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D _MainTex;

uniform float _Time;

uniform vec3 _WaterColor = vec3(0, 0.31, 0.85);
uniform float _S1 = 0.9;
uniform float _S2 = 0.03;
uniform float _Scale = 100;


void main()
{
	vec2 uv = TexCoords * _Scale + vec2(_Time);
	uv.x += (sin(uv.y + _Time) + sin(uv.y * 2 + _Time * 0.01) + sin(uv.y * 0.6 + _Time * 1.01)) / 5.0;
	uv.y += (sin(uv.x + _Time) + sin(uv.x * 2 + _Time * 0.02) + sin(uv.x * 0.27 + _Time * 1.61)) / 5.0;
	vec4 smp1 = texture(_MainTex, uv * 1).rgba;
	vec4 smp2 = texture(_MainTex, uv * 1 + vec2(0.2)).rgba;
	FragColor = vec4(_WaterColor + vec3(smp1.r * _S1 - smp2.r * _S2), 1.0);
}