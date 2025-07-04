#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outPos;
layout (location = 2) out vec3 outNor;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNor;
layout (location = 2) in vec2 inUv;


layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;
layout(push_constant) uniform PushConstants{ mat4 M;};


void main() {
    vec4 pos = camera.viewProj * M * vec4(inPos, 1.f);
    outPos = inPos;
    outNor = inNor;
    outUV = vec2(inUv.x, 1.f - inUv.y);
    gl_Position = pos;
}