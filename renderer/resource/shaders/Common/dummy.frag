#version 460

layout(location = 0) out vec4 outColor;

void main() {
    vec2 uv = gl_FragCoord.xy * 0.001;
    float v = 0.0;

    for (int i = 0; i < 500; ++i) {
        v += sin(uv.x + float(i) * 0.01) * cos(uv.y + float(i) * 0.01);
    }
    v /= 500.0;

    vec3 base = vec3(0.15, 0.15, 0.15); // #262626

    float grad = gl_FragCoord.y / 2560.0;
    vec3 color = mix(base * 0.9, base * 1.05, grad);

    color += 0.04 * v;

    outColor = vec4(color, 1.0);
}
