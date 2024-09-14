//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELSTRUCT_H
#define VKCELSHADINGRENDERER_MODELSTRUCT_H

#include "Engine/RHI/vkAsset.h"
//#include "Engine/ECS/Attachment/Scene.h"

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

//    struct Mesh{
//        std::vector<Vertex> vertices;
//        std::vector<uint32_t> indices;
//        std::shared_ptr<yic::vkBuffer> vertBuf;
//        std::shared_ptr<yic::vkBuffer> indexBuf;
//        uint8_t texIndex;
//        AABB aabb;
//    };

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
//        std::shared_ptr<yic::vkAccel>* pTlas;
    };
    struct Textures{
        std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
    };
    struct Descriptors{
        std::shared_ptr<yic::Descriptor> descriptor;
//        std::shared_ptr<yic::Descriptor>* pRtDescriptor;
    };

    struct Model{
        Info info;
        Mesh mesh;
        Textures texs;
        Descriptors des;
        AccelerationStructures as;
        vk::CommandBuffer cmd;
        void* pNext{};
    };


//    struct Model{
//        struct Generic : public Resource{
//            std::string id;
//            std::vector<Mesh> meshes;
//            std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
//            std::shared_ptr<yic::Descriptor> descriptor;
//            std::shared_ptr<yic::Descriptor> rtDescriptor;
//            vk::CommandBuffer cmd;
//            AABB aabb;
//            std::vector<std::string> shaderPaths;
//            std::string path;
//            std::vector<MeshBufAddress> meshesAddr;
//            std::shared_ptr<yic::vkBuffer> meshesAddrBuf;
//            std::vector<std::shared_ptr<yic::vkAccel>> blass;
//            std::shared_ptr<yic::vkAccel> tlas;
//        };
//
//        struct Pmx : public Resource{
//            std::string id;
//            std::shared_ptr<saba::PMXModel> pmx;
//            std::shared_ptr<saba::VMDAnimation> vmd;
//            std::vector<uint8_t> texIndex;
//            std::shared_ptr<yic::vkBuffer> vertBuf;
//            std::shared_ptr<yic::vkBuffer> indexBuf;
//            std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
//            std::shared_ptr<yic::Descriptor> descriptor;
//            std::vector<std::string> shaderPaths;
//
//            double saveTime, animTime;
//        };
//    };


}

#endif //VKCELSHADINGRENDERER_MODELSTRUCT_H


//    struct Model : public Resource{
//        std::string id;
//        std::vector<Mesh> meshes;
//        std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
//        std::shared_ptr<yic::Descriptor> descriptor;
//        std::vector<vk::CommandBuffer> cmds;
//        AABB aabb;
//        std::vector<std::string> shaderPaths;
//        std::string path;
//    };
//
//    struct Pmx : public Resource{
//        std::string id;
//        std::shared_ptr<saba::PMXModel> pmx;
//        std::shared_ptr<saba::VMDAnimation> vmd;
//        std::vector<uint8_t> texIndex;
//        std::shared_ptr<yic::vkBuffer> vertBuf;
//        std::shared_ptr<yic::vkBuffer> indexBuf;
//        std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
//        std::shared_ptr<yic::Descriptor> descriptor;
//        std::vector<std::string> shaderPaths;
//    };
