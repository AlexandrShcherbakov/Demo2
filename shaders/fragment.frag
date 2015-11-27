#version 330 core

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

struct LightSource {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec3 spotDirection;
    vec3 spotPosition;
    float spotCosCutoff;
    float innerConeCos;
};

in vec3 pos;
in vec3 normal;
in Material mat;

out vec4 finalColor;

uniform vec4 sceneColor;
uniform LightSource lg;
uniform vec3 viewer;

void main(void)
{
    finalColor = (sceneColor * mat.ambient) + (lg.ambient * mat.ambient);

    vec3 L = -normalize(pos - lg.spotPosition);
    vec3 D = normalize(lg.spotDirection);

	float current_angle = dot(-L, D);

	if (current_angle > lg.spotCosCutoff) {
		float spot = clamp((lg.spotCosCutoff - current_angle) / (lg.spotCosCutoff - lg.innerConeCos), 0.0f, 1.0f);
		vec3 N = normalize(normal);
		float lambertTerm = dot(N, L);
		if (lambertTerm > 0.0) {
			finalColor += lg.diffuse * mat.diffuse * lambertTerm * spot;
			vec3 E = normalize(viewer);
			vec3 R = reflect(-L, N);
			float specular = pow(max(dot(R, E), 0), mat.shininess);
			finalColor += lg.specular * mat.specular * specular * spot;
		}
	}
}
