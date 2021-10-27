#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require

precision mediump float;

in vec2 vTexCoords;
uniform samplerExternalOES uTexture;
out vec4 FragColor;

void main() {
    FragColor = texture(uTexture, vTexCoords);
}