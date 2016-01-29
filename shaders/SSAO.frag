#version 330 core

out vec4 finalColor;

vec4 EncodeShadow(float z) {
    vec4 res;
    res.x = z * (1 << 16);
    res.x -= floor(res.x);
    res.y = z * (1 << 8);
    res.y -= floor(res.y);
    res.z = z;
    res.w = 1;
    res.x = z;
    return res;
}

void main(void)
{
    finalColor = EncodeShadow(gl_FragCoord.z);
}
