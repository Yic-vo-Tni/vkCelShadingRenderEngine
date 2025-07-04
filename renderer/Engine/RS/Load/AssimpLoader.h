//
// Created by lenovo on 6/15/2025.
//

#ifndef VKCELSHADINGRENDERER_ASSIMPLOADER_H
#define VKCELSHADINGRENDERER_ASSIMPLOADER_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Utils/Auxiliary.h"
#include "Node.h"
#include "Animation.h"

namespace rs {

    class AssimpLoader {
        struct EdgeEntry {
            uint32_t opp0 = UINT32_MAX;
            uint32_t opp1 = UINT32_MAX;
        };
    public:
        struct ImportContext{
            std::shared_ptr<Assimp::Importer> importer;
            const aiScene* scene;
            const vot::string fileName;
            const vot::string pt;
            vot::unordered_map<std::filesystem::path, uint32_t> pts;
        };

    public:
        explicit AssimpLoader();
        ~AssimpLoader();

        auto Load(const vot::string &pt, vot::BasicInfoComponent& basicInfoComponent,
                       vot::VertexDataComponent &vertexDataComponent,
                       vot::RenderComponent &renderComponent, vot::AnimationComponent &animationComponent) -> vot::string;
    private:
        auto extractMesh(ImportContext& mic, vot::VertexDataComponent& vc, vot::RenderComponent& rc, vot::AnimationComponent& ac) -> void;
        auto extractAnim(ImportContext& mic, vot::AnimationComponent& ac) -> void;
    private:
        auto importScene(const vot::string& pt) -> ImportContext;
        auto extractCenter(const ImportContext& ctx, vot::RenderComponent& rc) -> void;
        auto assignBuffer(const ImportContext& ctx, vot::VertexDataComponent& vc) -> void;
        auto extractVertex(const aiMesh* aiMesh, const uint32_t& vertexOffset, vot::VertexDataComponent& vc) -> void;
        auto extractBone(const aiMesh* aiMesh, const uint32_t& vertexOffset, vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void;
        auto extractIndex(const aiMesh* aiMesh, const uint32_t& vertexOffset, const uint32_t& indexOffset, vot::VertexDataComponent& vc) -> void;
        auto extractDiffTex(ImportContext& ctx, const aiMaterial* aiMat, const vot::SubMesh& subMesh, vot::RenderComponent& rc) -> void;
        auto extractBoneNode(const ImportContext& ctx, vot::AnimationComponent& ac) -> void;
        auto buildAdjacencyIndex(const uint32_t* indexChunkFirst, uint32_t offset, uint32_t* adjIndexChunk) -> void;
    private:
        auto resolvingPath(const vot::string& pt) -> std::shared_ptr<void>;
        auto processMeshCenter(const auto* scene) -> glm::vec3;

        inline auto makeEdgeKey(uint32_t a, uint32_t b) -> uint64_t ;

        static inline glm::mat4 computeNodeGlobalTransform(const aiNode* node) {
            if (!node->mParent) {
                return miku::AssimpGLMConverter(node->mTransformation);
            }
            glm::mat4 parentGlobal = computeNodeGlobalTransform(node->mParent);
            glm::mat4 local = miku::AssimpGLMConverter(node->mTransformation);
            return parentGlobal * local;
        }

    private:
        ev::pVkSetupContext ct{};

        std::pmr::unsynchronized_pool_resource mVertexPool;
        std::pmr::unsynchronized_pool_resource mIndexPool;
        std::pmr::unsynchronized_pool_resource mAdjacencyIndexPool;
    };

} // rs

#endif //VKCELSHADINGRENDERER_ASSIMPLOADER_H
