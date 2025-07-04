#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNor;
layout(location = 2) out uint vIndex;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;

void main(){
    vec4 vpPos = camera.viewProj * vec4(inPos, 1.f);
    //outPos = inPos;
    outPos = inPos;
    //outNor = inNor;
    outNor = inNor;
    vIndex = gl_VertexIndex;
    //gl_Position = camera.viewProj * vec4(inPos, 1.f);
    gl_Position = vpPos;
}

