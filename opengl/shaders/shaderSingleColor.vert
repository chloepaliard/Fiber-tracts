#version 130

uniform mat4 camera_modelview;
uniform mat4 camera_projection;

in vec4 position;

out vec4 vertexToFragmentColor;

void main(void)
{
    gl_Position = camera_projection * camera_modelview * position;
}
