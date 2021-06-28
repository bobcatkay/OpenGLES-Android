#version 300 es

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aColor;

uniform mat4 uMatrix;
out vec2 vTexCoords;
out vec3 vColor;

void main() {
    gl_Position = vec4(aPos, 1.0f);
    vTexCoords = aTexCoords;
    vColor = aColor;
}