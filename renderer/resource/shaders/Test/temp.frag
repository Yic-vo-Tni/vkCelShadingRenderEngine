#version 460

#extension GL_GOOGLE_include_directive : enable

layout (location = 0) out vec4 outColor;

layout (set = 1, binding = 1) uniform sampler2D texSampler;

void main() {
    outColor = vec4(1.f);
}