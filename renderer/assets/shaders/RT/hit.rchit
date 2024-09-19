#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;
layout(location = 2) rayPayloadEXT bool shadowed;
hitAttributeEXT vec2 attribs;

layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
layout(binding = 2, set = 0) uniform camera{
    mat4 vp;
    mat4 viewInverse;
    mat4 projInverse;
} cam;

struct ObjDesc{
    uint64_t vertAddr;
    uint64_t indexAddr;
};

layout(buffer_reference, scalar) buffer Vertices { vec4 v[]; };
layout(buffer_reference, scalar) buffer Indices { uint i[]; };
layout(set = 0, binding = 3) buffer ObjDesc_ { ObjDesc i[]; } objDesc;

struct Vertex{
    vec3 pos;
    vec3 nor;
    vec2 uv;
};

Vertex unpack(Vertices vertices, uint index){
    Vertex v;

    const int m = 2;

    vec4 d0 = vertices.v[m * index];
    vec4 d1 = vertices.v[m * index + 1];

    v.pos = d0.xyz;
    v.nor = vec3(d0.w, d1.x, d1.y);

    return v;
}

void main()
{
    ObjDesc objResource = objDesc.i[gl_InstanceCustomIndexEXT];
    Vertices vertices = Vertices(objResource.vertAddr);
    Indices indices = Indices(objResource.indexAddr);

    uint id = gl_PrimitiveID * 3;
    Vertex v0 = unpack(vertices, id);
    Vertex v1 = unpack(vertices, id + 1);
    Vertex v2 = unpack(vertices, id + 2);

    vec3 lightPos = vec3(35.f, 15.f, 10.f);
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    vec3 nor = normalize(v0.nor * barycentricCoords.x + v1.nor * barycentricCoords.y + v2.nor * barycentricCoords.z);

    vec3 lightVec = normalize(lightPos.xyz);
    hitValue = vec3(1.f, 1.f, 1.f);

    float tmin = 0.001f;
    float tmax = 10000.f;
    vec3 orgin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    shadowed = true;

    traceRayEXT(tlas,
                gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT,
                0xFF,
                1,
                0,
                1,
                orgin,
                tmin,
                lightVec,
                tmax,
                2);

    if(shadowed){
        hitValue *= 0.3;
    }
}
