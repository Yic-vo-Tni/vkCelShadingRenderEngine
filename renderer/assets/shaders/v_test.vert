#version 450
layout (location = 0) out vec2 fragUv;

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNor;
layout (location = 2) in vec2 inUv;

layout (push_constant) uniform PushConstants {
    mat4 vpMatrix;
} pushConstants;

void main() {
    gl_Position = pushConstants.vpMatrix * vec4(inPos, 1.f);
    fragUv = inUv;
}