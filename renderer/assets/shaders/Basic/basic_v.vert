#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../host_device.glsl"

layout (location = 0) out vec2 fragUv;
layout (location = 1) out vec3 outPos;
layout (location = 2) out vec3 outNor;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNor;
layout (location = 2) in vec2 inUv;

layout(set = 0, binding = 0) uniform _GlobalUniforms { GlobalUniforms uni; };

void main() {
    outPos = inPos;
    outNor = inNor;
    gl_Position = uni.viewProj * vec4(inPos, 1.f);
    fragUv = inUv;
}