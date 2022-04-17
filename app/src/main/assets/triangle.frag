#version 300 es

precision mediump float;

in vec3 vColor;
out vec4 FragColor;

uniform float time;

float cosFunc(float offset);

void main() {
    vec4 appendColor = vec4(cosFunc(0.0), cosFunc(1.0), cosFunc(2.0), cosFunc(3.0));
    FragColor = vec4(vColor, 1.0) + appendColor;
    FragColor = normalize(FragColor);
}

float cosFunc(float offset) {
    return cos(time + offset) / 2.0 + 0.5;
}