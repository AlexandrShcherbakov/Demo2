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
in vec4 posForLight;
in vec4 fragRadio;

out vec4 finalColor;

uniform vec4 sceneColor;
uniform LightSource lg;
uniform vec3 viewer;
uniform sampler2D shadowMap;

float DecodeShadow(vec4 f) {
    return f.x;
    return f.x / (1 << 16) + f.y / (1 << 8)+ f.z;
}

vec4 gamma(vec4 v) {
    return vec4(pow(v.xyz, vec3(1.0f / 2.2f)), v.w);
}

void main(void)
{
    //finalColor = (sceneColor * mat.ambient) + (lg.ambient * mat.ambient);
    finalColor = fragRadio;

    vec3 lightProj = posForLight.xyz / posForLight.w / 2 + vec3(0.5f);
    //lightProj.z = posForLight.z / posForLight.w / 2 + 0.5;
    float shadowCoef = 0.0f;
    for (float x_off = -1.5f; x_off <= 1.5f; x_off += 1.0f) {
        for (float y_off = -1.5f; y_off <= 1.5f; y_off += 1.0f) {
            if (DecodeShadow(texture(shadowMap, lightProj.xy + vec2(x_off, y_off) / 1024)) >= lightProj.z - 0.00007f) {
                shadowCoef += 1.0f;
            }
        }
    }
	shadowCoef /= 16.0f;
	if (shadowCoef > 0.0f) {
		vec3 L = -normalize(pos - lg.spotPosition);
		vec3 D = normalize(lg.spotDirection);

		float current_angle = dot(-L, D);

		if (current_angle > lg.spotCosCutoff) {
			float spot = clamp((lg.spotCosCutoff - current_angle) / (lg.spotCosCutoff - lg.innerConeCos), 0.0f, 1.0f);
			vec3 N = normalize(normal);
			float lambertTerm = dot(N, L);
			if (lambertTerm > 0.0) {
				finalColor += lg.diffuse * mat.diffuse * lambertTerm * spot * shadowCoef;
				vec3 E = normalize(viewer);
				vec3 R = reflect(-L, N);
				float specular = pow(max(dot(R, E), 0), mat.shininess);
				finalColor += lg.specular * mat.specular * specular * spot * shadowCoef;
			}
		}
	}

	//finalColor = fragRadio;
	if (length(lg.spotPosition - pos) < 0.01f) finalColor = vec4(1);
	finalColor = gamma(finalColor);
	//finalColor = vec4(lightProj.z);
	//finalColor = vec4(DecodeShadow(texture(shadowMap, gl_FragCoord.xy / vec2(800, 600))));
	//finalColor = texture(shadowMap, gl_FragCoord.xy / vec2(800, 600));
	//finalColor = -vec4(DecodeShadow(texture(shadowMap, lightProj.xy)) - lightProj.z) * 20;
}
