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

struct DescriptorSet0{ vot::DescriptorHandle handles[3]; };

// struct Vertex {
//     glm::vec3 pos;
//     glm::vec3 nor;
//     glm::vec2 uv;
//
//     glm::ivec4 boneIds{-1};
//     glm::vec4 boneWeight{0.f};
// };
//
// struct MMDVertex {
//     glm::vec3 pos;
//     glm::vec3 nor;
//     glm::vec2 uv;
// };

enum VertexType {
    eMMD = 0, eAssimp = 1
};

template<VertexType Type>
struct VertexT {
    glm::vec3 pos;
    glm::vec3 nor;
    glm::vec2 uv;

    std::conditional_t<Type == VertexType::eAssimp, glm::ivec4, std::monostate> boneIds;
    std::conditional_t<Type == VertexType::eAssimp, glm::vec4, std::monostate> boneWeight;
};

// struct VertexSkinning : public VertexT {
//     glm::ivec4 boneIds{-1};
//     glm::vec4 boneWeight{0.f};
// };



// template<bool HasBones>
// struct VertexT{
//     glm::vec3 pos{};
//     glm::vec3 nor{};
//     glm::vec2 uv{};
//
//     std::conditional_t<HasBones, glm::ivec4, std::monostate> boneIds{-1};
//     std::conditional_t<HasBones, glm::vec4, std::monostate> boneWeight{0.f};
// };

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

struct VertexDataComponent {
    bool isMMD{false};
    std::array<std::pmr::vector<VertexT<eAssimp> >, 3> vertices_pmr;
    std::array<std::pmr::vector<VertexT<eMMD> >, 3> mmdVertices_pmr;
    std::pmr::vector<uint32_t> indices_pmr;
    std::pmr::vector<uint32_t> adjIndices_pmr;
    std::shared_ptr<saba::MMDModel> pmx = nullptr;
};
// template<VertexType Type>
// struct VertexDataComponent {
//     std::array<std::pmr::vector<VertexT<Type> >, 3> vertices_pmr;
//     std::pmr::vector<uint32_t> indices_pmr;
//     std::pmr::vector<uint32_t> adjIndices_pmr;
//     std::shared_ptr<saba::MMDModel> pmx = nullptr;
// };
//
// using VertexDataComponentVariant = std::variant<
//     VertexDataComponent<VertexType::eAssimp>,
//     VertexDataComponent<VertexType::eMMD>
//     >;

struct RenderComponent{
    glm::mat4 baseMat{1.f};
    glm::mat4 zmoMat{1.f};
    glm::vec3 center;
    vot::Buffer_sptr vertexBuffer[3];
    vot::Buffer_sptr indexBuffer;
    vot::Buffer_sptr adjIndexBuffer;
    vot::vector<vot::Image_sptr> diffuseTextures;
    vot::DescriptorHandle dsHandle;
    vot::unordered_map<uint8_t, vot::vector<SubMesh>> subMeshes;
    vk::IndexType indexType{vk::IndexType::eUint32};
    vk::CommandBuffer* cmd; // unused
};


struct AnimationComponent{
    int boneCount;
    float animTime{0.f};
    bool enableAnim{false};
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


}

#endif //VKCELSHADINGRENDERER_RS_STRUCT_H
