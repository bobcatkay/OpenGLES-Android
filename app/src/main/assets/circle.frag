#version 300 es

precision mediump float;

uniform vec2 uSurfaceSize;
out vec4 FragColor;

void main() {
    vec2 uv = gl_FragCoord.xy/uSurfaceSize * 2.0 - vec2(1.0);
    float aspect = uSurfaceSize.x / uSurfaceSize.y;

    if (aspect > 1.0) {
        uv.x *= aspect;
    } else {
        uv.y /= aspect;
    }

    uv *= 2.0;

    float fade = 0.005;
    float thickness = 0.1;
    uv.y += (1.0 - fade * 2.0);

    FragColor.rg = uv;
    FragColor.b = 0.0;

    float distance = 1.0 - length(uv);
    vec3 color = vec3(smoothstep(0.0, fade, distance));
    color *= vec3(1.0 - smoothstep(thickness - fade, thickness, distance));
    FragColor.rgb = color;
}