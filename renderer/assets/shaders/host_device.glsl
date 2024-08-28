//
// Created by lenovo on 8/11/2024.
//

//#ifndef VKCELSHADINGRENDERER_HOST_DEVICE_H
//#define VKCELSHADINGRENDERER_HOST_DEVICE_H



struct Vertex{
    vec3 pos;
    vec3 nor;
    vec2 uv;
};

struct GlobalUniforms{
    mat4 viewProj;
    mat4 viewInverse;
    mat4 projInverse;
};

struct PushConstantRay{
    vec4 clearColor;
    vec3 lightPosition;
    float lightIntensity;
    int lightType;
};

struct ObjDesc{
    uint64_t vertAddr;
    uint64_t indexAddr;
};


//#endif //VKCELSHADINGRENDERER_HOST_DEVICE_H
