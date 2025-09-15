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

enum VertexType {
    eMMD = 0, eAssimp = 1
};

template<VertexType Type>
struct VertexT {
    glm::vec3 pos;
    glm::vec3 nor;
    glm::vec2 uv;

    std::conditional_t<Type == eAssimp, glm::ivec4, std::monostate> boneIds;
    std::conditional_t<Type == eAssimp, glm::vec4, std::monostate> boneWeight;
};

    struct LoadOptions {
        std::optional<float> scale = std::nullopt;
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

struct VertexDataComponent {
    enum pmr_type{eTPose = 0, eAnim = 1, eCount = 2};
    vot::VertexType type;
    std::array<std::pmr::vector<VertexT<eAssimp> >, eCount> vertices_pmr;
    std::array<std::pmr::vector<VertexT<eMMD> >, eCount> mmd_vertices_pmr;
    std::pmr::vector<uint32_t> indices_pmr;
    std::pmr::vector<uint32_t> adjIndices_pmr;
    std::shared_ptr<saba::MMDModel> pmx = nullptr;
};

struct RenderComponent{
    glm::mat4 baseMat{1.f};
    glm::mat4 zmoMat{1.f};
    glm::vec3 center;
    //vot::Buffer_sptr vertexBuffer[3];
    std::array<vot::Buffer_sptr, 3> vertexBuffer;
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
    std::array<vot::vector<glm::mat4>, 3> boneMats;
    vot::Buffer_sptr boneMatBuffer;
    BoneNode boneNode;
};

struct RayTracingComponent{
    vot::Accel_sptr blas;

    vot::Buffer_sptr scratchBuffer;
};

    struct PointLightComponent {

    };


}

namespace vot::comp::Light {
    struct Directional {
        glm::vec4 direction;
        glm::vec4 color;
        glm::vec4 intensity;
    };
    struct Point {
        glm::vec4 pos{0.f, 0.f, 0.f, 1.f};
        glm::vec4 color{1.f, 1.f, 1.f, 1.f};
        glm::vec4 intensity_radius_constant_linear{1.f, 25.f, 1.f, 0.09f};
        glm::vec4 quadratic_3pad{0.032f, 0.f, 0.f, 0.f};
    };
    struct Spot {

    };

    struct Meta {
        uint32_t count{0};
        vot::Buffer_sptr buffer;
    };
    struct Array {
        vot::vector<vot::comp::Light::Point> points;
        vot::Buffer_sptr buffer;
    };
}

#endif //VKCELSHADINGRENDERER_RS_STRUCT_H
