#version 130

uniform vec3 singleColor;
out vec4 color;

void main(void)
{
    color = vec4(singleColor, 1.0);
}
