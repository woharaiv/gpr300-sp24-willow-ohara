#version 450
out vec4 FragColor;

uniform sampler2D _Scene;
in vec2 UV;

void main(){
	FragColor = texture(_Scene, UV);
}
