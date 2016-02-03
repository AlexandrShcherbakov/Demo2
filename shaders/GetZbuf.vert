#version 330 core

in vec3 position;

uniform mat4 camMatrix;

void main(void)
{
    gl_Position = camMatrix * vec4(position, 1.0);
}
