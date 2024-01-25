#version 450
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{ 
    vec3 color = texture(screenTexture, TexCoords).rgb;

    color.rb *= (sin(TexCoords.y*textureSize(screenTexture,0).y) + 1) * 0.1 + 1;
    color.g *= (sin(TexCoords.y*textureSize(screenTexture,0).y) + 1) * 0.15 + 1;
        
    FragColor = vec4(color, 1.0);
}
