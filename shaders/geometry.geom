#version 330 core

uniform mat4 projectionMatrix;
uniform vec3 norm[6];

smooth in vec3 pos[];
smooth out vec3 posit;

layout(lines_adjacency) in;
layout(triangle_strip, max_vertices = 4) out;

in vec4 gColor[];
in uint polInd[];
out vec4 fColor;
out vec3 fnormal;

void main(void) {
    fnormal = norm[polInd[0]];

    fColor = gColor[0];
    posit = (pos[0] + pos[1] + pos[2] + pos[3]) / 4.0f;
    gl_Position = projectionMatrix * gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = projectionMatrix * gl_in[1].gl_Position;
    EmitVertex();
    gl_Position = projectionMatrix * gl_in[3].gl_Position;
    EmitVertex();
    gl_Position = projectionMatrix * gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
