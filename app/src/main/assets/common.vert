#version 300 es

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

uniform mat4 uTransform;
uniform mat4 uProjection;
out vec2 vTexCoords;

void main() {
    gl_Position = uProjection * uTransform * vec4(aPos, 1.0);
    vTexCoords = aTexCoords;
}