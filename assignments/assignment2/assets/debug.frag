#version 450

out vec4 FragColor;
in vec2 uv;

uniform sampler2D debug_image;

void main()
{
	float depth = texture(debug_image, uv).r;
	FragColor = vec4(vec3(depth), 1.0);
}