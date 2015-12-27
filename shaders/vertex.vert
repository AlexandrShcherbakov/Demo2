#version 330 core

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

in vec4 position;
in uint v_polInd;
in vec4 radioPart;

out vec3 normal;
out vec3 pos;
out Material mat;
out vec4 posForLight;
out vec4 fragRadio;

uniform vec3 norm[50];

uniform mat4 camMatrix;
uniform mat4 lightMatrix;
uniform Material maters[50];

vec4 gamma(vec4 v) {
    return vec4(pow(v.xyz, vec3(1.0f / 2.2f)), v.w);
}

void main(void)
{
    gl_Position = camMatrix * vec4(position.xyz, 1.0);
    normal = norm[v_polInd];
    /*mat.ambient = gamma(maters[v_polInd].ambient);
    mat.diffuse = gamma(maters[v_polInd].diffuse);
    mat.specular = gamma(maters[v_polInd].specular);*/
    mat.ambient = maters[v_polInd].ambient;
    mat.diffuse = maters[v_polInd].diffuse;
    mat.specular = maters[v_polInd].specular;
    mat.shininess = maters[v_polInd].shininess;
    pos = position.xyz;
    posForLight = lightMatrix * vec4(position.xyz, 1.0);
    fragRadio = radioPart;
}
