#version 460

#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "../common.glsl"

layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;

layout(input_attachment_index = 0, set = 1, binding = 0) uniform subpassInput i_gAlbedo;
layout(input_attachment_index = 1, set = 1, binding = 1) uniform subpassInput i_gPos;
layout(input_attachment_index = 2, set = 1, binding = 2) uniform subpassInput i_gNor;
layout(input_attachment_index = 3, set = 1, binding = 3) uniform subpassInput i_Volumetric_fog;
//layout(input_attachment_index = 4, set = 1, binding = 4) uniform subpassInput i_Volumetric_clouds;
layout(set = 1, binding = 4) uniform sampler2D clouds;
layout(set = 1, binding = 5) uniform sampler2D raytracing;
layout(set = 1, binding = 6) uniform sampler2D blueNoise;

vec3 Tonemap_ACES(const vec3 x)
{
    // ACES approximation
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}


void main()
{
    vec2 uv = outUV;
    vec3 cameraPos = camera.pos_pad.xyz;

    vec4 albedo = subpassLoad(i_gAlbedo);
    vec3 pos = subpassLoad(i_gPos).xyz;
    vec3 nor = normalize(subpassLoad(i_gNor).xyz);
 //   vec4 volumetric_clouds = subpassLoad(i_Volumetric_clouds);
    vec3 cloud = texture(clouds, uv).rgb;
    vec4 volumetric_fog = subpassLoad(i_Volumetric_fog);
    vec4 rt = texture(raytracing, uv).rgba;


    vec3 viewDir = normalize(cameraPos - pos);
    vec3 lightDir = normalize(vec3(7.f, 3.f, 2.f));

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = dot(nor, halfwayDir);
    float edge = 0.97f;  // 0.95~0.99越高越小块
    float toonSpec = smoothstep(edge, edge+0.015, NdotH); // 软边版，也可以直接step

    vec3 toonSpecular = toonSpec * 0.1f * vec3(0.92, 0.68f, 0.1f);

    float lightIntensity = max(dot(lightDir, nor), 0.0);

    rt.rgb = clamp(rt.rgb + 0.05f, 0.45f, 1.05f);
    float shadowBands = 3.f;
    vec3 quantShaodw = floor(rt.rgb * shadowBands) / (shadowBands - 1.f);
    quantShaodw = mix(vec3(0.35f), vec3(1.f), quantShaodw);

    vec3 baseColor = albedo.rgb * rt.rgb + toonSpecular;

    // vloumetric fog
    float dist = distance(cameraPos, pos);
    if(false){
        float ao = 0.70f;
        baseColor = mix(baseColor, baseColor * ao, 0.35);

        lightIntensity = max(dot(lightDir, viewDir), 0.0);
        lightIntensity = pow(lightIntensity, 2);

        // float fogFactor = 1.f - exp(-0.005f * dist);
        float k = 0.0006f;
        float fogFactor = 1.f - exp(-k * pow(dist, 1.5f));
        fogFactor += lightIntensity * 0.25f;
        fogFactor = clamp(fogFactor, 0.f, 1.f);

        float layerStep = 10.f;
        float heightBands = floor(pos.y / layerStep);
        float bandFactor = heightBands * layerStep / 200.f;
        bandFactor = pow(bandFactor, 1.2f);

        float heightFactor = smoothstep(0.f, 200.f, pos.y);

        vec3 fogColor = mix(vec3(0.98, 0.84, 0.74), vec3(1.0, 0.80, 0.68), heightFactor);

        float toSun = dot(normalize(pos - cameraPos), lightDir);
        float godray = smoothstep(0.5f, 1.f, toSun);

        fogColor += godray * 0.6f * vec3(1.f, 0.85f, 0.5f);

        float fogAlpha = clamp(1.0 - fogFactor, 0.0, 0.4);

        const vec2 noiseUV = outUV / textureSize(blueNoise, 0);
        float blueNoiseVal = texture(blueNoise, noiseUV).r;
        float fogNoiseStrength = 0.06f;
        vec3 fogColorJitter = fogColor * (1.f + fogNoiseStrength * (blueNoiseVal - 0.5f) * 2.f);
        float fogFactorJitter = clamp(fogFactor + fogNoiseStrength * (blueNoiseVal - 0.5), 0.0, 1.0);

        baseColor = mix(baseColor, fogColor, fogFactorJitter);
    } else {
        const int steps = 128;
        float t_start = 0.0;
        float t_end = dist;
        float dt = (t_end - t_start) / float(steps);

        vec3 rayDir = normalize(pos - cameraPos);
        vec3 rayStart = cameraPos;
        vec3 accumFog = vec3(0.0);
        float accumAlpha = 0.0;

        for (int i = 0; i < steps; ++i) {
            float t = t_start + dt * (i + 0.5);
            vec3 samplePos = rayStart + rayDir * t;

           // float heightFog = clamp(1.f - samplePos.y / 200.f, 0.f, 1.f);
            float baseDensity = exp(-samplePos.y * 0.02f); // +
            float fogStart = 0.f;
            float fogEng = 4000.f;
            float distFactor = clamp((t - fogStart) / (fogEng - fogStart), 0.012f, 0.8f);
            //float distFactor = 1.f - exp(-t * 0.001f); // +

            vec2 noiseUV = fract(samplePos.xz * 0.02);
            float blueNoiseVal = texture(blueNoise, noiseUV).r;

            float noiseFacotr = 0.8f + 0.2f * blueNoiseVal; //+

            float localDensity = baseDensity * distFactor * noiseFacotr;

            vec3 sunDir = normalize(vec3(7.f, 3.f, 2.f));
            float toSun = dot(normalize(sunDir), normalize(samplePos - cameraPos));
            float godrayStrength = pow(max(toSun, 0.f), 12.f);
            vec3 godrayColor = vec3(1.f, 0.92f, 0.65f);

            vec3 fogColor = vec3(0.98f, 0.84f, 0.74f) * (1.f - samplePos.y / 200.f);
            fogColor += godrayColor * godrayStrength;

            float stepAlpha = 1.f - exp(-localDensity * dt);
            accumFog += (1.f - accumAlpha) * fogColor * stepAlpha;
            accumAlpha += (1.f - accumAlpha) * stepAlpha;
            if(accumAlpha > 0.98) break;

        }
        float fogWeight = clamp(accumAlpha, 0.f, 1.f);
        fogWeight *= mix(0.8f, 0.4f, rt.r);
        baseColor = mix(baseColor, accumFog, fogWeight);

//        baseColor = mix(baseColor, accumFog, clamp(accumAlpha, 0.0, 1.0));

    }



    if(albedo.a < 0.99f){
        baseColor = cloud;
    }

  // float exposure = 0.8f;
   // baseColor = clamp(baseColor * exposure, 0.0, 6.0);
   // baseColor = Tonemap_ACES(baseColor * exposure);

  //  baseColor = pow(baseColor, vec3(1.f / 2.2));

   // baseColor = pow(baseColor, vec3(1.1f));

    float saturation = 1.4f;
    float lum = dot(baseColor, vec3(0.299f, 0.587f, 0.114f));
    baseColor = mix(vec3(lum), baseColor, saturation);

    fragColor = vec4(baseColor, 1.f);
}