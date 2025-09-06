#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants{ mat4 LightMvpMat;};

//layout(set = 0, binding = 0) uniform UBO {
//    mat4 lightMVPMatrix;
//} ubo;


void main() {
    vec4 worldPosition = vec4(inPosition, 1.0);
    gl_Position = LightMvpMat * worldPosition;
}