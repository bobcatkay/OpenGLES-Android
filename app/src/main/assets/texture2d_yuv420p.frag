#version 300 es

precision mediump float;

uniform sampler2D yTex;
uniform sampler2D uTex;
uniform sampler2D vTex;
in vec2 vTexCoords;
out vec4 FragColor;

const mat3 CONVERT_MAT = mat3(1.0,     1.0,         1.0,
                              0.0,     -0.39465,    2.03211,
                              1.13983, -0.58060,    0.0);

void main()
{
    vec3 yuv;
    vec3 rgb;
    yuv.r = texture(yTex,vTexCoords).r - 0.0625;
    yuv.g = texture(uTex,vTexCoords).r - 0.5;
    yuv.b = texture(vTex,vTexCoords).r - 0.5;
    rgb = CONVERT_MAT*yuv;

    yuv.r = 0;
    FragColor = vec4(rgb, 1.0);
}