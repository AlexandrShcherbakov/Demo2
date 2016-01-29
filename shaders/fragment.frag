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
uniform sampler2D SSAOtex;
uniform mat4 camMatrix;

float DecodeShadow(vec4 f) {
    return f.x;
    return f.x / (1 << 16) + f.y / (1 << 8)+ f.z;
}

vec4 gamma(vec4 v) {
    return vec4(pow(v.xyz, vec3(1.0f / 2.2f)), v.w);
}


void main(void)
{
    //finalColor = (sceneColor * mat.ambient) / 10 + (lg.ambient * mat.ambient) / 10;
    finalColor = fragRadio;

    vec3 lightProj = posForLight.xyz / posForLight.w / 2 + vec3(0.5f);
    //lightProj.z = posForLight.z / posForLight.w / 2 + 0.5;
    float shadowCoef = 0.0f;
    for (float x_off = -1.5f; x_off <= 1.5f; x_off += 1.0f) {
        for (float y_off = -1.5f; y_off <= 1.5f; y_off += 1.0f) {
            if (DecodeShadow(texture(shadowMap, lightProj.xy + vec2(x_off, y_off) / 1024)) >= lightProj.z - 0.0007f) {
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
	//finalColor = vec4(vec3(texture(SSAOtex, gl_FragCoord.xy / vec2(800, 600)).x / 2), 1);

	//SSAO
	vec4 rndTable[8] = vec4[8] (
		vec4 ( -0.5, -0.5, -0.5, 0.0 ),
		vec4 (  0.5, -0.5, -0.5, 0.0 ),
		vec4 ( -0.5,  0.5, -0.5, 0.0 ),
		vec4 (  0.5,  0.5, -0.5, 0.0 ),
		vec4 ( -0.5, -0.5,  0.5, 0.0 ),
		vec4 (  0.5, -0.5,  0.5, 0.0 ),
		vec4 ( -0.5,  0.5,  0.5, 0.0 ),
		vec4 (  0.5,  0.5,  0.5, 0.0 )
	);

	float depth = 1.0f / texture(SSAOtex, gl_FragCoord.xy / vec2(800, 600)).x;
    float AO = 0.0f;
    float radius = 0.2f * 40;
    float attBias = 0.45f;
    float attScale = 1.0f;
    float distScale = 1.0f;

	for (int i = 0; i < 8; ++i) {
		vec3 sample = reflect(rndTable[i].xyz, normal);
        float zSample = 1.0f / texture(SSAOtex, (gl_FragCoord.xy + radius * sample.xy / depth) / vec2(800, 600)).x;

        if (zSample - depth > 0.1f) continue;

        //float dz = max(zSample - depth, 0.0f) * 30.0f * 600;
        float dist = max(zSample - depth, 0.0f) / distScale;
        float occl = 15 * max(dist * (2.0f - dist), 0.0f);

        AO += 1.0f / (1.0f + occl * occl * 600 * 600);
	}

	AO = clamp((AO / 8.0f + attBias) * attScale, 0.0f, 1.0f);

	finalColor = gamma(finalColor);
	finalColor *= vec4(AO);

	//finalColor = vec4(depth);

	//finalColor = vec4(lightProj.z);
	//finalColor = vec4(DecodeShadow(texture(shadowMap, gl_FragCoord.xy / vec2(800, 600))));
	//finalColor = texture(shadowMap, gl_FragCoord.xy / vec2(800, 600));
	//finalColor = -vec4(DecodeShadow(texture(shadowMap, lightProj.xy)) - lightProj.z) * 20;
}
