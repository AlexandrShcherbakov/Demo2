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
uniform sampler2D Ztex;
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
    finalColor = vec4(0);//fragRadio;

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

	if (length(lg.spotPosition - pos) < 0.01f) finalColor = vec4(1);

	float   h1 = 0.5;
    float   h2 = 0.5;
    vec4    ao = vec4(0.0);

    /*for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            ao += texture(SSAOtex, (gl_FragCoord.xy + vec2((2 * i + 1) * h1, (2 * j + 1) * h2)) / vec2(800, 600));

	ao /= 9.0f;*/
	ao = texture(SSAOtex, (gl_FragCoord.xy) / vec2(800, 600));
	finalColor += fragRadio * ao;
	//finalColor = ao;

	finalColor = gamma(finalColor);

    //gl_FragColor = texture( srcMap, gl_TexCoord [0].xy ) * pow ( ao / 9.0, vec4 ( 2.0 ) );


	//finalColor = vec4(texture(Ztex, gl_FragCoord.xy / vec2(800, 600)));

	//finalColor = vec4(depth);

	//finalColor = vec4(lightProj.z);
	//finalColor = vec4(DecodeShadow(texture(shadowMap, gl_FragCoord.xy / vec2(800, 600))));
	//finalColor = texture(shadowMap, gl_FragCoord.xy / vec2(800, 600));
	//finalColor = -vec4(DecodeShadow(texture(shadowMap, lightProj.xy)) - lightProj.z) * 20;
}
