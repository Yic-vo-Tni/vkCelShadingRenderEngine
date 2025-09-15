#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec3 inPos;

layout (set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; };
layout (set = 0, binding = 1, std140) uniform LightMeta {
    uint count;
};
//layout (set = 0, binding = 2) uniform GlobalUniforms {};

layout(push_constant) uniform PushConstants{ mat4 M;};

void main(){
    vec4 pos = (vec4(inPos * 0.4f, 1.f));
    pos = camera.viewProj * M * pos;
    gl_Position = pos;
    //gl_Position = vec4(inPos, 1.f);
}