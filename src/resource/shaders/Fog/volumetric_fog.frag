#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;
layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput i_gPos;
layout(set = 1, binding = 1) uniform sampler2D shadowMap;
layout(set = 1, binding = 2) uniform sampler2D u_blueNoise;

layout(push_constant) uniform PushConstants{ mat4 lightMat;};

const vec2 iResolution = vec2(2560, 1440);
const int steps = 128;
float intensity = .025f;
const float noiseOffset = 1.f;
const float u_beerPower = 1.f;
const float u_powderPower = 1.f;

float shadow(vec4 lightSpacePos){
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
  //  if(projCoords.z > 1.f) return 1.f;
    projCoords = projCoords * 0.5f + 0.5f;
//    if(any(lessThan(projCoords.xy, vec2(0))) || any(greaterThan(projCoords.xy, vec2(1)))){
//        return 1.f;
//    }

    float closestDepth = texture(shadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;
    float shadow = currentDepth + 0.005f > closestDepth ? 1.f : 0.f;

    return shadow;
}

void main(){
//    const vec3 rayEnd = subpassLoad(i_gPos).xyz;
//    const vec3 rayStart = camera.pos_pad.xyz;
//    const vec3 rayDir = normalize(rayEnd - rayStart);
//
//    const float totalDistance = distance(rayStart, rayEnd);
//    const float rayStep = totalDistance / steps;
//    const vec2 noiseUV = gl_FragCoord.xy / textureSize(u_blueNoise, 0);
//    vec3 rayPos = rayStart + rayDir * rayStep * texture(u_blueNoise, noiseUV).x * noiseOffset;
//
//    float accum = 0.f;
//    float maxDist = 200.f;
//    for(int i = 0; i < steps; i++){
//        vec4 lightSpacePos = lightMat * vec4(rayPos, 1.f);
//        accum += 1.f - shadow(lightSpacePos);
//        rayPos += rayDir * rayStep;
//    }
//
//    float noise = texture(u_blueNoise, (rayEnd.xz * 0.02) + noiseUV).r;
//    intensity *= mix(0.7f, 1.2f, noise);
//    float d = accum * rayStep * intensity;
//    d *= mix(0.3f, 2.5f, noise);
//    float powder = 1.f - exp(-d * 2.f * u_powderPower);
//    float beer = exp(-d * u_beerPower);
//
//    float fogIndentity = (1.f - beer) * powder;
//    vec3 fogColor = mix(vec3(0.99f, 0.72f, 0.07f), vec3(0.98f, 0.81f, 0.69f), noise);
//    fragColor = vec4(0.8f, 0.75f, 0.65f, fogIndentity);
    fragColor = vec4(0.f, 0.f, 0.f, 0.f);
}