#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct ray{
    vec4 direction;
    vec4 origin;

    bool in_use;

    vec3 normal; //命中物体表面法线

    int child_reflect_id;
    int child_refract_id; //
    int parent_id; //父光线索引（由另一条光线产生）

    float base_color;
    float accumulated_color; //光线路径累计颜色

    float reflection_constant; //反射常数
    float refraction_constant; //

    bool external_reflection_ray;
    bool external_refraction_ray; //是否是外部折射

    int level; //深度
};