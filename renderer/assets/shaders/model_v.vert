#version 450

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "host_device.glsl"

layout (location = 0) out vec2 fragUv;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNor;
layout (location = 2) in vec2 inUv;

layout(set = 0, binding = 0) uniform _GlobalUniforms { GlobalUniforms uni; };
//layout(set = 0, binding = 0) uniform Camera{
//    mat4 vpMatrix;
//} camera;

void main() {
    gl_Position = uni.viewProj * vec4(inPos, 1.f);
    fragUv = inUv;
}