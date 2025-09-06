#version 460

layout(push_constant) uniform PushConstants {
    mat4 model;
    uint objectID;
} pc;

layout(location = 0) out uint outObjectID;

void main() {
    outObjectID = pc.objectID;
}





