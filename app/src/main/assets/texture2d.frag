#version 300 es

precision mediump float;

in vec2 vTexCoords;
uniform sampler2D uTexture;
out vec4 FragColor;

void main() {
    FragColor = texture(uTexture, vTexCoords);
//    FragColor = vec4(0, 1.0, 0, 1.0);
}