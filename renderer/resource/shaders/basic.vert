#version 460

layout(location = 0) out vec3 fragColor;

void main(){
    vec2 pos[3] = vec2[](vec2(0.f, -.5f), vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
    vec3 col[3] = vec3[](vec3(1.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f), vec3(0.f, 0.f, 1.f));

    gl_Position = vec4(pos[gl_VertexIndex], 0.f, 1.f);
    fragColor = col[gl_VertexIndex];
}