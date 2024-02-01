#version 450
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float curvature;

void main()
{ 
    vec2 uv = (TexCoords * 2) - 1;
    vec2 offset = uv.yx / curvature;
    uv = uv + uv * offset * offset;
    uv = (uv * 0.5) + 0.5;
    vec3 color = int(0 < uv.x && uv.x < 1 && 0 < uv.y && uv.y < 1) * texture(screenTexture, uv).rgb;
    FragColor = vec4(color, 1.0);
}
