#version 450

//layout(location = 0) in vec3 edgeColor;  // 从几何着色器传递的边缘颜色

layout(location = 0) out vec4 fragColor; // 输出颜色

void main() {
  //  fragColor = vec4(edgeColor, 1.0);  // 输出边缘的颜色为黑色
    fragColor = vec4(1.f, 1.f, 1.f, 1.0);  // 输出边缘的颜色为黑色
}