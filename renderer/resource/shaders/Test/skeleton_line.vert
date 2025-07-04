#version 450

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec3 inPos;
layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;

void main(){
    gl_Position = camera.viewProj * vec4(inPos, 1.f);
}