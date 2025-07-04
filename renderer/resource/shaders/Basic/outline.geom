#version 450

#extension GL_GOOGLE_include_directive: enable

#include "../common.glsl"

layout (triangles_adjacency) in;
layout (line_strip, max_vertices = 4) out;

layout (location = 0) in vec3 inPos[];
layout (location = 1) in vec3 inNor[];
layout (location = 2) in uint vIndex[];

bool isFront(vec4 A, vec4 B, vec4 C){
    return 0 < ( A.x * B.y - B.x * A.y) * C.w
            + (B.x * C.y - C.x * B.y) * A.w
            + (C.x * A.y - A.x * C.y) * B.w;
}

vec3 getNorm(vec3 A, vec3 B, vec3 C){
    return normalize(cross(B - A, C - A));
}

void emitEdge(vec4 p0, vec4 p1){
    gl_Position = p0; EmitVertex();
    gl_Position = p1; EmitVertex();

    EndPrimitive();
}

void main(){
    vec4 v0 = gl_in[0].gl_Position;
    vec4 v1 = gl_in[1].gl_Position;
    vec4 v2 = gl_in[2].gl_Position;
    vec4 v3 = gl_in[3].gl_Position;
    vec4 v4 = gl_in[4].gl_Position;
    vec4 v5 = gl_in[5].gl_Position;

    if(!isFront(v0, v2, v4)) return;

    vec3 fn0 = getNorm(inPos[0], inPos[2], inPos[4]);
    vec3 fn1 = getNorm(inPos[0], inPos[1], inPos[2]);
    vec3 fn2 = getNorm(inPos[2], inPos[3], inPos[4]);
    vec3 fn3 = getNorm(inPos[4], inPos[5], inPos[0]);

    if (!isFront(v0, v1, v2) || dot(fn0, fn1) < 0.5f) emitEdge(v0, v2);
    if (!isFront(v2, v3, v4) || dot(fn0, fn2) < 0.5f) emitEdge(v2, v4);
    if (!isFront(v4, v5, v0) || dot(fn0, fn3) < 0.5f) emitEdge(v4, v0);
}

//layout(set = 0, binding = 0) uniform GlobalUniforms { CameraMatrixUniform camera; } ;

//bool isBoundary(int e){
//    return vIndex[e * 2 + 1] == vIndex[e * 2 + 0];
//}
//vec3 faceNormal(int v0i, int v1i, int v2i){
//    vec3 A = inPos[v0i], B = inPos[v1i], C = inPos[v2i];
//    return normalize(cross(B - A, C - A));
//}
//
//
//bool isSilhouette(int e) {
//    vec3 n0 = inNor[edgeIdx * 2 + 0];
//    vec3 n1 = inNor[edgeIdx * 2 + 1];
//    //vec3 viewDir = normalize(camera.pos - inPos[edgeIdx * 2 + 0]);
//    vec3 viewDir = normalize(vec3(0, 0, 25.f) - inPos[edgeIdx * 2 + 0]);
//    float d0 = dot(n0, viewDir);
//    float d1 = dot(n1, viewDir);
//    return d0 * d1 < 0.f;
////    if(isBoundary(e)) return true;
////
////    vec3 fn0 = faceNormal(e * 2 + 0, e * 2 + 1, e * 2 + 2);
////    vec3 fn1 = faceNormal(e * 2 + 1, e * 2 + 0, e * 2 + 3);
////
////    vec3 viewDir = normalize(vec3(0.f, 0.f, 25.f) - inPos[e * 2 + 0]);
////    if(dot(fn0, viewDir) * dot(fn1, viewDir) < 0.f) return true;
////    if(dot(fn0, fn1) < 0.5f) return true;
////
////    return false;
//}
//
//void main() {
//    for (int e = 0; e < 3; e++) {
//        if (!isSilhouette(e))
//        continue;
//
//        vec4 p0 = gl_in[e * 2 + 0].gl_Position;
//        vec4 p1 = gl_in[e * 2 + 1].gl_Position;
//
//        //        vec3 dir = normalize(vWorldPos[e*2 + 0] - cam.cameraPos);
//        //        p0.xyz += dir * pc.edgeWidth;
//        //        p1.xyz += dir * pc.edgeWidth;
//
//        gl_Position = p0; EmitVertex();
//        gl_Position = p1; EmitVertex();
//        EndPrimitive();
//
//    }
//}


//layout(triangles) in;
//layout(triangle_strip, max_vertices = 6) out;
//
//layout(location = 0) in vec3 inNor[];
//
//layout(location = 0) out vec3 edgeColor;
//
//void main(){
//    float edgeThinckness = 0.1f;
//    for(int i = 0; i < 3; i++){
//        vec3 offset = inNor[i] * edgeThinckness;
//
//        gl_Position = gl_in[i].gl_Position + vec4(offset, 0.f);
//        edgeColor = vec3(0.f);
//        EmitVertex();
//
//        gl_Position = gl_in[(i + 1) % 3].gl_Position + vec4(offset, 0.0);
//        edgeColor = vec3(0.0, 0.0, 0.0);
//        EmitVertex();
//
//        gl_Position = gl_in[(i + 2) % 3].gl_Position + vec4(offset, 0.0);
//        edgeColor = vec3(0.0, 0.0, 0.0);
//        EmitVertex();
//
//        EndPrimitive();
//    }
//}