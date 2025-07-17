#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput i_gAlbedo;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput i_gPos;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput i_Volumetric_fog;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput i_Volumetric_clouds;
layout(set = 1, binding = 4) uniform sampler2D raytracing;

void main()
{
    vec2 uv = outUV;
    vec3 cameraPos = camera.pos_pad.xyz;
    //    float gamma = 1.f / 2.2f;
    //    fragColor   = pow(texture(noisyTxt, uv).rgba, vec4(gamma));

    vec4 albedo = subpassLoad(i_gAlbedo);
    vec3 pos = subpassLoad(i_gPos).xyz;
    vec4 volumetric_clouds = subpassLoad(i_Volumetric_clouds);
    vec4 volumetric_fog = subpassLoad(i_Volumetric_fog);
    vec4 rt = texture(raytracing, uv).rgba;

    vec3 viewDir = normalize(cameraPos - pos);
    vec3 lightDir = normalize(vec3(-0.5f, 0.4f, -0.5f));

    float lightIntensity = max(dot(lightDir, viewDir), 0.0);
    lightIntensity = pow(lightIntensity, 2);

    float depth = pos.z;
    float fogFactor = exp(-0.05f * depth);
    fogFactor += lightIntensity * 0.25f;
    fogFactor = clamp(fogFactor, 0.f, 1.f);

    float heightFactor = smoothstep(0.f, 200.f, pos.y);
    vec3 fogColor = mix(vec3(0.99, 0.72, 0.07), vec3(0.98, 0.81, 0.69), heightFactor);

    albedo = mix(albedo, vec4(fogColor, albedo.a), 1.f - fogFactor);

    if(albedo.a > 0.99f){
        vec4 color = albedo * rt;
        fragColor.rgb = mix(color.rgb, volumetric_fog.rgb, volumetric_fog.a);
        fragColor.a = 1.f;
    } else {
        fragColor = volumetric_clouds;
    }

}