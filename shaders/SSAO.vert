#version 330 core

in vec4 position;
//in uint v_polInd;

//out vec3 normal;
out vec3 pos;

//uniform vec3 norm[50];

uniform mat4 camMatrix;


void main(void)
{
    gl_Position = camMatrix * vec4(position.xyz, 1.0);
    //normal = norm[v_polInd];
    pos = position.xyz;
}
