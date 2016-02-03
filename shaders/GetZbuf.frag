#version 330 core

out vec4 finalColor;

void main(void)
{
    finalColor.x = gl_FragCoord.z;
}
