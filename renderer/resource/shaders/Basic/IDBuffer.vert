#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout (location = 0) in vec3 inPos;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;
layout(push_constant) uniform PushConstants{
    mat4 M;
    uint ID;
};

void main() {
    vec4 pos = camera.viewProj * M * vec4(inPos, 1.f);
    gl_Position = pos;
}