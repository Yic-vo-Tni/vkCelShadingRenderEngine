//
// Created by lenovo on 10/31/2024.
//

#ifndef VKCELSHADINGRENDERER_RS_STRUCT_H
#define VKCELSHADINGRENDERER_RS_STRUCT_H

#include "pch.h"
#include "stl_mimalloc.h"
#include "rhi_struct.h"
#include "ozz/base/memory/unique_ptr.h"
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "MikuMikuFormats/Vmd.h"
#include <Saba/Model/MMD/PMXModel.h>

namespace rs{ template<typename T> class MemoryChunk; }
template<typename T>
using MemoryChunk = rs::MemoryChunk<T>;

namespace rs{ class Animation; }

namespace vot::inline rs{

struct Vertex {
    glm::vec3 pos;
    glm::vec3 nor;
    glm::vec2 uv;

    glm::ivec4 boneIds{-1};
    glm::vec4 boneWeight{0.f};
};



struct AABB {
    glm::vec3 max;
    glm::vec3 min;
};

struct BoneInfo{
    int id;
    glm::mat4 offset;
};

struct BoneNode
{
    glm::mat4 transformation;
    vot::string name;
    uint32_t childrenCount;
    std::vector<BoneNode> children;
};

struct SubMesh {
    uint32_t indexCount;
    uint32_t firstIndex;
};

struct BasicInfoComponent{
    vot::string name;
    vot::string animationName;
    bool playAnimation = false;
};

struct VertexDataComponent{
    bool isMMD{false};
    std::pmr::vector<Vertex> vertices_pmr;
    std::pmr::vector<uint32_t> indices_pmr;
    std::pmr::vector<uint32_t> adjIndices_pmr;
    std::shared_ptr<saba::MMDModel> pmx = nullptr;
};

struct RenderComponent{
    glm::mat4 baseMat{1.f};
    glm::mat4 zmoMat{1.f};
    glm::vec3 center;
    vot::Buffer_sptr vertexBuffer;
    vot::Buffer_sptr indexBuffer;
    vot::Buffer_sptr adjIndexBuffer;
    vot::vector<vot::Image_sptr> diffuseTextures;
    vot::DescriptorHandle dsHandle;
    vot::unordered_map<uint8_t, vot::vector<SubMesh>> subMeshes;
    vk::IndexType indexType;
    vk::CommandBuffer* cmd; // unused
};


struct AnimationComponent{
    int boneCount;
    uint8_t activeAnim{0};
    vot::map<vot::string, BoneInfo> boneMap;
    vot::vector<std::pair<vot::string, std::shared_ptr<::rs::Animation>>> animations;
    std::pair<vot::string, std::shared_ptr<saba::VMDAnimation>> vmd;
    vot::vector<glm::mat4> boneMats;
    vot::Buffer_sptr boneMatBuffer;
    BoneNode boneNode;
};

struct RayTracingComponent{
    vot::Accel_sptr blas;

    vot::Buffer_sptr scratchBuffer;
};

struct RenderVisibleTag{ };
struct MMDTag{};

}

#endif //VKCELSHADINGRENDERER_RS_STRUCT_H
