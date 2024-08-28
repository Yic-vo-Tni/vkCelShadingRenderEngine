#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "raycommon.glsl"
#include "wavefront.glsl"

hitAttributeEXT vec2 hitAttri;

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(buffer_reference) buffer Vertices { Vertex v[]; };
layout(buffer_reference) buffer Indices { ivec3 i[]; };
layout(set = 0, binding = 0) uniform accelerationStructureEXT TLAS;
layout(set = 0, binding = 2) uniform _GlobalUniforms { GlobalUniforms uni; };  // 全局统一变量
layout(set = 0, binding = 3) buffer ObjDesc_ { ObjDesc i[]; } objDesc;

layout(push_constant) uniform _PushConstantRay { PushConstantRay pcRay;};

void main() {
    ObjDesc objResource = objDesc.i[gl_InstanceCustomIndexEXT];
    Indices indices = Indices(objResource.indexAddr);
    Vertices vertices = Vertices(objResource.vertAddr);

    ivec3 ind = indices.i[gl_PrimitiveID];

    Vertex v0 = vertices.v[ind.x];
    Vertex v1 = vertices.v[ind.y];
    Vertex v2 = vertices.v[ind.z];

    const vec3 barycentries = vec3(1.f - hitAttri.x - hitAttri.y, hitAttri.x, hitAttri.y); // 重心坐标

    const vec3 pos = v0.pos * barycentries.x + v1.pos * barycentries.y + v2.pos * barycentries.z;
    const vec3 worldPs = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.f));

    const vec3 nor = v0.nor * barycentries.x + v1.nor * barycentries.y + v2.nor * barycentries.z;
    const vec3 worldNor = normalize(vec3(nor * gl_WorldToObjectEXT));

        vec3 L;
        float lightIntensity = pcRay.lightIntensity;
        float lightDistance = 100000.0;

        if (pcRay.lightType == 0){
            vec3 lDir      = pcRay.lightPosition - worldPs;
            lightDistance  = length(lDir);
            lightIntensity = pcRay.lightIntensity / (lightDistance * lightDistance);
            L              = normalize(lDir);
        } else{
            L = normalize(pcRay.lightPosition);
        }

//        if(dot(worldNor, L) > 0){
//            float tMin = 0.001;
//            float tMax = lightDistance;
//            vec3 origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
//            // origin : 光线起点在世界坐标中的位置
//            // dir : 用于确定光线在3D空间中的路径
//            // hit : 表示光线从起点到第一个击中点的参数t, 用于计算实际击中位置，origin用于后续光线的发射和计算
//            vec3 rayDir = L;
//            uint flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
//            // onfirst : 找到第一个物体后停止追踪该光线
//            // opaque : 只与不透明物体交互，忽略半透明或透明物体
//            // skip : 检测到阻挡跳过最近击中着色器执行
//            isShadowed = true;
//            traceRayEXT(TLAS,
//                        flags,
//                        0xFF,
//                        0,
//                        0,
//                        1,
//                        origin,
//                        tMin,
//                        rayDir,
//                        tMax,
//                        1
//            );
//    }



    float attenuation = 1.f;

    if(isShadowed){
        attenuation = 0.3f;
    }

    vec3 hitValue = lightIntensity * attenuation * vec3(1.f);

//    prd.hitValue = vec3(lightIntensity * attenuation * vec3(1.f));
}
