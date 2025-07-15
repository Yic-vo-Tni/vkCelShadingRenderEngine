#version 460

#extension GL_GOOGLE_include_directive : enable

//#include "../PBR/basic/pbr.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inPos;
layout (location = 2) in vec3 inNor;

layout (set = 1, binding = 0) uniform sampler2D texSampler;

layout (location = 0) out vec4 gAlbedo;
layout (location = 1) out vec4 gPos;

void main() {
    vec3 color = vec3(1.f);
    float a = 0.f;

    vec4 diff = texture(texSampler, inUV).rgba;

    if (diff.a < 1.f) {
        //discard;
    } else {
        color = vec3(diff.r, diff.g, diff.b);
        a = 1.f;
    }

    gAlbedo = vec4(color, a);
    gPos = vec4(inPos, 1.f);
}
