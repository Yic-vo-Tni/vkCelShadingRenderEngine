#version 460

#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable

//layout(set = 0, binding = 0) uniform GlobalUniform{
//    CameraMatrixUniform camera;
//} ;
//layout(set = 1, binding = 0) uniform accelerationStructureEXT tlas;
//layout(set = 1, binding = 1) uniform image2D image;
//
//layout(location = 0) rayPayloadEXT vec3 hitValue ;
//
//void main(){
//    const vec2 pixelCenter = vec2(gl_lanunchIDEXT.xy) + vec2(0.5f);
//    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
//
//    vec2 d = inUV * 2.f - 1.f;
//
//    traceRayEXT(    tlas,
//                    gl_RayFlagsOpaqueEXT,
//                    0xff,
//                    0,
//                    0,
//                    0,
//                    origin.xyz,
//                    0.01f,
//                    dir.xyz,
//                    10000.f,
//                    0);
//
//
//
//}
void main(){

}