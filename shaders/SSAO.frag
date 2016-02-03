#version 330 core

in vec3 pos;
//in vec3 normal;

out vec4 finalColor;

uniform sampler2D Ztex;

void main(void)
{

	//finalColor = vec4(1);
	//return;
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

	vec3 planeTable[16] = vec3[16] (
		vec3( 0.2692,  0.1946,  0.1946),
		vec3( 0.0874, -0.6378, -0.3246),
		vec3(-0.7566, -0.2410, -0.1914),
		vec3(-0.7806,  0.9232,  0.6226),
		vec3( 0.0520,  0.6568,  0.8920),
		vec3( 0.2196, -0.0908, -0.1484),
		vec3( 0.4492, -0.3710, -0.3440),
		vec3(-0.9984, -0.1810,  0.9894),
		vec3(-0.0161, -0.3695, -0.6799),
		vec3( 0.6672, -0.7318,  0.3976),
		vec3( 0.5627, -0.0085,  0.9331),
		vec3( 0.8813, -0.6571, -0.8303),
		vec3( 0.9279, -0.6428, -0.6344),
		vec3(-0.9738, -0.3602, -0.9216),
		vec3( 0.0664,  0.8741,  0.9952),
		vec3( 0.3817,  0.6321, -0.3027)
	);

	float radius = 0.2f * 10;
    float zFar = 5.0f;
    float zNear = 0.5f;
    float attBias = 0.45f;
    float attScale = 1.0f;
    float distScale = 1.0f;


	float depth = texture(Ztex, gl_FragCoord.xy / vec2(800, 600)).x;
	float z = zFar * zNear / (depth * (zFar - zNear) - zFar);
	vec3 pe = pos * z / pos.z;
    float AO = 0.0f;
    int pCoord = int(gl_FragCoord.x) % 4 + int(gl_FragCoord.y) % 4 * 4;
    vec3 plane = 2.0f * planeTable[pCoord] - vec3(1.0f);

	for (int i = 0; i < 8; ++i) {
		vec3 sample = reflect(rndTable[i].xyz, plane);
        float zSample = texture(Ztex, (gl_FragCoord.xy + radius * sample.xy / z) / vec2(800, 600)).x;

        zSample = zFar * zNear / (zSample * (zFar - zNear) - zFar);

        float dist = max(zSample - z, 0.0f) / distScale;
        float occl = 15 * max(dist * (2.0f - dist), 0.0f);

        AO += 1.0f / (1.0f + occl * occl * 800 * 800);
	}

	AO = clamp((AO / 8.0f + attBias) * attScale, 0.0f, 1.0f);

	finalColor = vec4(AO);
	//finalColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}
