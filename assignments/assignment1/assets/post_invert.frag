#version 450
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
    vec3 color = texture(screenTexture, TexCoords).rgb;
    color = 1 - color;
    FragColor = vec4(color, 1.0);
}
