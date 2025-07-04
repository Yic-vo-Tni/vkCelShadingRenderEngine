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
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 boneWeight;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;
layout(push_constant) uniform PushConstants{ mat4 M;};

layout(std430, set = 1, binding = 1) readonly buffer BoneStorage {
    mat4 boneMatrices[];
};


vec4 applyBoneTransform(vec4 pos){
    vec4 r = vec4(0.f);
    for(int i = 0; i < 4; i++){

        int id = boneIds[i];
        if (id < 0) continue;
        r += boneWeight[i] * (boneMatrices[boneIds[i]] * pos);
    }
    if( r == vec4(0.f)){
        return pos;
    }
    return r;
}

void main() {
//    vec4 vpPos = camera.viewProj * vec4(inPos, 1.f);
//    outPos = vec3(vpPos.x, vpPos.y, vpPos.z);
//    outNor = inNor;
//    outUV = inUv;
//    gl_Position = vpPos;

    vec4 pos = applyBoneTransform(vec4(inPos, 1.f));
    vec3 nor = normalize(applyBoneTransform(vec4(inNor, 1.f))).xyz;
    pos = camera.viewProj * M * pos;
//    vec4 pos = camera.viewProj * M * vec4(inPos, 1.f);
    outPos = inPos;
    outNor = inNor;
    outUV = inUv;
    gl_Position = pos;
}