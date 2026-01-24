//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_RS2_H
#define VKCELSHADINGRENDERER_RS2_H

#include <type_traits>

#include "External/pch.h"
#include "../../../Foundation/stl_mimalloc.h"
#include "External/rhi_struct.h"


namespace rs2 {

    namespace details {
        template<typename T, typename Tag>
        struct Tagged {
            T value{};

            Tagged() = default;
            Tagged(const T& v) : value(v) {}

            operator T&() { return value; }
            operator const T&() const { return value; }

            Tagged& operator=(const T& v) {
                value = v;
                return *this;
            }
        };

        namespace hide {
            struct PositionTag {};
            struct NormalTag {};
            struct UVTag {};
            struct IndexTag {};
        }

        using Position3 = Tagged<glm::vec3, hide::PositionTag>;
        using Normal3 = Tagged<glm::vec3, hide::NormalTag>;
        using UV2 = Tagged<glm::vec2, hide::UVTag>;
        using IndexU32 = Tagged<uint32_t, hide::IndexTag>;
    }

    struct AABB {
        glm::vec3 min{};
        glm::vec3 max{};
    };

    enum class SkinningType {
        eLBS = 1,
        eSDEF = 2,
        eDQ = 3,
    };


    struct LBS {
        glm::ivec4 boneIds{-1};
        glm::vec4 boneWeights{0.f};
    };

    struct SDEF {
        glm::vec3 c, r0, r1;
    };

    struct DQ{
        glm::quat real, dual;
    };

    struct Skinning {
        SkinningType type{SkinningType::eLBS};
        int exIndex{-1};
    };


    struct Bounds {
        AABB aabb;
    };

    struct BoneNode {

    };

    struct PositionMorph {
        glm::vec3 pos;
        uint32_t delta;
    };

    struct UVMorph {
        uint32_t index;
        glm::vec4 delta;
    };

    struct MorphT {
        float weight{0.f};
        vot::vector<PositionMorph> positions;
    };

    struct UVMorphT {
        vot::vector<UVMorph> uvs;
    };

    struct SubMesh {
        uint32_t indexCount{};
        uint32_t firstIndex{};
    };

    struct VertexData {
        std::pmr::vector<details::Position3> positions;
        std::pmr::vector<details::Normal3> normals;
        std::pmr::vector<details::UV2> uvs;
        std::pmr::vector<details::Position3> updatePositions;
        std::pmr::vector<details::Normal3> updateNormals;
        std::pmr::vector<details::UV2> updateUvs;
        std::pmr::vector<details::IndexU32> indices_pmr;
        std::pmr::vector<details::IndexU32> adjIndices_pmr;
        std::pmr::vector<LBS> skinning_pmr;
        std::pmr::vector<SDEF> sdef_pmr;
        std::pmr::vector<DQ> dualQuaternion_pmr;
        std::pmr::vector<MorphT> morphs_pmr;
        std::pmr::vector<UVMorphT> uvmorphs_pmr;
        std::pmr::vector<BoneNode> boneNodes_pmr;
    };

    struct VertexBufferHandle{
        vot::Buffer_sptr pos;
        vot::Buffer_sptr normals;
        vot::Buffer_sptr uvs;
    };

    namespace comp_Mod {

        enum class Mark : uint32_t {
            eNone = 0,
            eRenderable = 1 << 0,
            Animatable = 1 << 1,
            Material = 1 << 2,
            Physical = 1 << 3,
            Collider = 1 << 4,
            RayTracing = 1 << 5,
            ColdRender = 1 << 6,
        };

        struct Meta { // hot
            vot::string name;
            vot::string path;
        };

        struct Attachment { // cold
            Mark mark{Mark::eNone};
            Bounds bounds;
            std::shared_ptr<VertexData> vertexData;
        };

        struct Transform {
            glm::mat4 baseMat{1.f};
            glm::mat4 zmoMat{1.f};
            glm::vec3 center;
        };

        struct Renderable {
            VertexBufferHandle vertexHandle;
            vot::Buffer_sptr indexHandle;
            vot::DescriptorHandle dsHandle;
            vot::unordered_map<uint32_t, vot::vector<SubMesh>> subMeshes;
            vk::IndexType indexType{vk::IndexType::eUint32};
        };

        struct RenderableCold {
            vot::Buffer_sptr adjIndex;
        };

        struct Animatable {
            uint8_t active : 7 {0};
            uint8_t play : 1 {0};
            VertexBufferHandle vertexHandleEx[2];

        };

        struct Material {

        };

        struct Physical {

        };

        struct Collider {

        };

        struct RayTracing {
            vot::Accel_sptr blas;
            vot::Buffer_sptr scratchBuffer;
        };
    }

}

#endif //VKCELSHADINGRENDERER_RS2_H