#version 460 core

// 输入是三角形
layout(triangles) in;

// 输出是三角形条带，最多 3 个顶点
layout(triangle_strip, max_vertices = 3) out;

// 接收 VS 的输出
layout(location = 0) in VS_OUT {
    vec3 normal;
} gs_in[];

// 传给 FS
layout(location = 0) out GS_OUT {
    vec3 normal;
} gs_out;

void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        gs_out.normal = gs_in[i].normal;
        EmitVertex();
    }
    EndPrimitive();
}
