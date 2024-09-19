//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELSTRUCT_H
#define VKCELSHADINGRENDERER_MODELSTRUCT_H

#include "Engine/RHI/vkAsset.h"
#include "Engine/Materials/Material.h"

namespace yic{
    class Descriptor;
}

namespace sc{

    struct Resource{
        ResFormat format;
    };

    struct Vertex{
        glm::vec3 pos;
        glm::vec3 nor;
        glm::vec2 uv;
    };

    struct AABB{
        glm::vec3 max;
        glm::vec3 min;
    };

    struct MeshBufAddress{
        uint64_t vertAddr;
        uint64_t indexAddr;
    };

    struct Info{
        std::string id;
        std::vector<std::string> shaderPts;
    };

    struct SubMesh{
        uint32_t indexCount;
        uint32_t firstIndex;
        uint8_t texIndex;
        AABB aabb;
    };
    struct Mesh{
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<SubMesh> subMeshes;
        std::shared_ptr<yic::vkBuffer> vertBuf;
        std::shared_ptr<yic::vkBuffer> indexBuf;
        MeshBufAddress meshBufAddress;
        AABB aabb;
    };
    struct AccelerationStructures{
        std::shared_ptr<yic::vkAccel> blas;
    };
    struct Textures{
        std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
    };
    struct Descriptors{
        std::shared_ptr<yic::Descriptor> descriptor;
    };

    struct Model{
        Info info;
        Mesh mesh;
        Textures texs;
        Descriptors des;
        AccelerationStructures as;
        mt::Material material;
        vk::CommandBuffer cmd;
        void* pNext{};
    };

}

#endif //VKCELSHADINGRENDERER_MODELSTRUCT_H

