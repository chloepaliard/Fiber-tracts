#version 130

uniform mat4 camera_modelview;
uniform mat4 camera_projection;
uniform float slider_position;

in vec4 position;
in vec4 color;
in vec4 newposition;

out vec4 vertexToFragmentColor;

void main(void)
{
	vertexToFragmentColor = color;
    gl_Position = camera_projection * camera_modelview * ((1-slider_position)*position + slider_position*newposition);
}
