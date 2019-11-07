#version 130

in vec4 vertexToFragmentColor;
out vec4 color;

void main(void)
{
	color = vertexToFragmentColor;
}
