#version 330 core
out vec4 FragColor;

uniform float uAlpha;
uniform vec4 uColor;

void main()
{
    FragColor = vec4(uColor.rgb, uAlpha * uColor.a);
}