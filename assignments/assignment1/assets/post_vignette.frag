#version 450
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float vignetteStrength;

void main()
{ 
    vec2 edgeBasedUV = 1- abs(TexCoords * 2 - 1);
    vec3 vignette = vec3((smoothstep(0, vignetteStrength*100/textureSize(screenTexture,0).x, edgeBasedUV.x) * smoothstep(0, vignetteStrength*100/textureSize(screenTexture,0).y, edgeBasedUV.y)));
    vec4 color = texture(screenTexture, TexCoords);
    FragColor = vec4((vec3(color) * vignette), 1.0);
}
