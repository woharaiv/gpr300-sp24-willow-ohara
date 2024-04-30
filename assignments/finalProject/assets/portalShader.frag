#version 450
out vec4 FragColor;

uniform sampler2D _Scene;
in vec2 UV;

void main(){
	//FragColor = vec4(UV, 0.0, 1.0);
	FragColor = texture(_Scene, UV);
}
