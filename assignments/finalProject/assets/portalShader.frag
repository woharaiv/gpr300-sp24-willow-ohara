#version 450
out vec4 FragColor;

uniform sampler2D _Scene;
uniform vec3 color;
in vec2 screenUV;


void main(){
	
	vec3 color = texture(_Scene, screenUV).rgb + color;

	FragColor = vec4(color,1.0);
}

