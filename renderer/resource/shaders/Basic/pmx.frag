#version 460

#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inPos;
layout (location = 2) in vec3 inNor;

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

void main() {
    vec3 color = vec3(1.f);
    float a = 0.f;

    vec4 diff = texture(texSampler, inUV).rgba;

    if(diff.a < 1.f){
        //discard;
    } else {
        color = vec3(diff.r, diff.g, diff.b);
        a = 1.f;
    }

    vec3 viewDir = normalize(vec3(0.f, 0.f, 25.f) - inPos);
    vec3 lightDir = normalize(vec3(-0.5f, 0.4f, -0.5f));

    float lightIntensity = max(dot(lightDir, viewDir), 0.0);
    lightIntensity = pow(lightIntensity, 2);

    float depth = inPos.z;
    float fogFactor = exp(-0.005f * depth);
    fogFactor += lightIntensity * 0.25f;
    fogFactor = clamp(fogFactor, 0.f, 1.f);

    float heightFactor = smoothstep(0.f, 200.f, inPos.y);
    vec3 fogColor = mix(vec3(0.99, 0.72, 0.07),vec3(0.98, 0.81, 0.69), heightFactor);

    color = mix(color, fogColor, 1.f - fogFactor);

    outColor = vec4(color, a);
}
