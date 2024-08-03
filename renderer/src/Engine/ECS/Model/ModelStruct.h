//
// Created by lenovo on 7/21/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELSTRUCT_H
#define VKCELSHADINGRENDERER_MODELSTRUCT_H

#include "Engine/RHI/vkAsset.h"

namespace yic{
    class Descriptor;
}

namespace sc{

    struct Vertex{
        glm::vec3 pos;
        glm::vec3 nor;
        glm::vec2 uv;
    };

    struct AABB{
        glm::vec3 max;
        glm::vec3 min;
    };

    struct Mesh{
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::shared_ptr<yic::vkBuffer> vertBuf;
        std::shared_ptr<yic::vkBuffer> indexBuf;
        uint8_t texIndex;
        AABB aabb;
    };

    struct Model{
        std::string id;
        std::vector<Mesh> meshes;
        std::vector<std::shared_ptr<yic::vkImage>> diffTexs;
//        oneapi::tbb::concurrent_vector<Mesh> meshes;
//        oneapi::tbb::concurrent_vector<std::shared_ptr<yic::vkImage>> diffTexs;
        std::shared_ptr<yic::Descriptor> descriptor;
        AABB aabb;
    };



}

#endif //VKCELSHADINGRENDERER_MODELSTRUCT_H
