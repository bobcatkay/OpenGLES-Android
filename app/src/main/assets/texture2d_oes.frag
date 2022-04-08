#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require

precision mediump float;

in vec2 vTexCoords;
uniform samplerExternalOES uTexture;
uniform int mRotation;
out vec4 FragColor;

void main() {
    float r = mRotation == 0 ? 0.2 : 0.0;
    float g = mRotation == 0 ? 0.0 : 0.2;
    vec4 mask = vec4(r, g, 0.0, 0.0);
    FragColor = texture(uTexture, vTexCoords) + mask;
}