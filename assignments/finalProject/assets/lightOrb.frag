#version 450
out vec4 FragColor;

uniform vec3 _Color = vec3(1.0);

void main(){
	FragColor = vec4(_Color,1.0);
}
