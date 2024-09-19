#version 460

#extension GL_GOOGLE_include_directive : enable

#include "../PBR/basic/pbr.glsl"

layout (location = 0) in vec2 fragUv;
layout (location = 1) in vec3 inPos;
layout (location = 2) in vec3 inNor;

layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main() {
    vec4 diff = texture(texSampler, fragUv).rgba;

    vec3 N = normalize(inNor);
    vec3 V = normalize(vec3(0.f, 0.f, 25.f) - inPos);

    float roughness = 0.2f;
    vec3 lightPos = vec3(35.f, 15.f, 10.f);

    //roughness = max(roughness, step(fract(inPos.y * 2.02), 0.5));

    vec3 Lo = vec3(0.f);
    vec3 L = normalize(lightPos);
    Lo += BRDF(L, V, N, diff.rgb, 0.1f, roughness);

    vec3 color = diff.rgb * 0.2f;
    Lo.x = max(0, Lo.x);
    Lo.y = max(0, Lo.y);
    Lo.z = max(0, Lo.z);
    color += Lo;

    color = pow(color, vec3(0.4545));

    if (diff.a < 1.f) {
        discard;
    } else {
       outColor = diff;
    }
}
