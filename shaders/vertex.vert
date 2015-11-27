#version 330 core

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

in vec3 position;
in uint v_polInd;

out vec3 normal;
out vec3 pos;
out Material mat;

uniform vec3 norm[50];
uniform mat4 projectionMatrix;
uniform mat4 shiftMatrix;
uniform mat4 rotateMatrix;
uniform Material maters[50];

vec4 gamma(vec4 v) {
    return vec4(pow(v.xyz, vec3(1.0f / 2.2f)), v.w);
}

void main(void)
{
    gl_Position = projectionMatrix * shiftMatrix * rotateMatrix * vec4(position, 1.0);
    normal = norm[v_polInd];
    mat.ambient = gamma(maters[v_polInd].ambient);
    mat.diffuse = gamma(maters[v_polInd].diffuse);
    mat.specular = gamma(maters[v_polInd].specular);
    mat.shininess = maters[v_polInd].shininess;
    pos = position;
}
