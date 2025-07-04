
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

struct CameraMatrixUniform{
    mat4 viewProj;
    mat4 viewInverse;
    mat4 projInverse;
};

struct ObjDesc{
    uint64_t vertAddress;
    uint64_t indexAddress;
};