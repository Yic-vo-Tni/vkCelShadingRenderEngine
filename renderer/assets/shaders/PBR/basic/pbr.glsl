
const float PI = 3.14159265359;

struct Material {
    vec4 albedo;
    vec4 roughness_metallic_specular_opacity;
    vec4 emissive;
};

float D_GGX(float dotNH, float roughness){
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = dotNH * dotNH * (alpha2 - 1.f) + 1.f;
    return (alpha2) /(PI * denom * denom);
}

float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness){
    float r = (roughness + 1.f);
    float k = (r * r) / 8.f;
    float GL = dotNL / (dotNL * (1.f - k) + k);
    float GV = dotNV / (dotNV * (1.f - k) + k);
    return GL * GV;
}

vec3 F_Schlick(float cosTheta, float metallic, vec3 albedo){
    vec3 F0 = mix(vec3(0.04f), albedo, metallic);
    vec3 F = F0 + (1.f - F0) * pow(1.f - cosTheta, 5.f);
    return F;
}

vec3 BRDF(vec3 L, vec3 V, vec3 N, vec3 albedo, float metallic, float roughness){
    vec3 H = normalize(V + L);
    float dotNV = clamp(dot(N, V), 0.f, 1.f);
    float dotNL = clamp(dot(N, L), 0.f, 1.f);
    float dotLH = clamp(dot(L, H), 0.f, 1.f);
    float dotNH = clamp(dot(N, H), 0.f, 1.f);

    vec3 lightColor = vec3(1.f);

    vec3 color = vec3(0.f);

    if(dotNL > 0.f){
        float rroughness = max(0.05f, roughness);
        float D = D_GGX(dotNH, roughness);
        float G = G_SchlicksmithGGX(dotNL, dotNV, rroughness);
        vec3 F = F_Schlick(dotNV, metallic, albedo);

        vec3 spec = D * F * G / (4.f * dotNL * dotNV);

        color += spec * dotNL * lightColor;
    }
    return color;
}

