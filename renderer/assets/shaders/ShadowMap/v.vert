#version 460

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConstants{
    mat4 lightMVPMatrix;
} push;

void main(){
    vec4 worldPos = vec4(inPosition, 1.f);
    gl_Position = push.lightMVPMatrix * worldPos;
}