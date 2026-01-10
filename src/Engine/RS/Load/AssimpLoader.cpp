//
// Created by lenovo on 6/15/2025.
//

#include "AssimpLoader.h"

#include "Utils/FileOperation.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "RHI/Command.h"
#include "RHI/DescriptorSystem.h"
#include "Runtime/System/RenderLibrary.h"

namespace rs {

    AssimpLoader::AssimpLoader() {
        ct = yic::systemHub.va<ev::pVkSetupContext>();

        defaultTex = yic::allocator->loadTexture(tex_path "icon.jpg");
    }

    AssimpLoader::~AssimpLoader() = default;

    auto AssimpLoader::Load(const vot::string &pt, vot::BasicInfoComponent &basicInfoComponent,
                                 vot::VertexDataComponent &vertexDataComponent,
                                 vot::RenderComponent &renderComponent, vot::AnimationComponent &animationComponent, const vot::LoadOptions& options) -> vot::string {
        auto ctx = importScene(pt);

        extractMesh(ctx, vertexDataComponent, renderComponent, animationComponent, options);
        extractAnim(ctx, animationComponent);

        return basicInfoComponent.name = ctx.fileName;
    }

    auto AssimpLoader::importScene(const vot::string &pt) -> ImportContext {
        auto importer = std::make_shared<Assimp::Importer>();
        const auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_LimitBoneWeights | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            throw std::runtime_error("assimp load model error: " + std::string (importer->GetErrorString()));
        const std::filesystem::path p(pt);

        return {.importer = std::move(importer), .scene = scene,
                .fileName = p.stem().string().data(), .pt = pt, };
    }

    auto AssimpLoader::extractMesh(ImportContext& mic, vot::VertexDataComponent& vc, vot::RenderComponent& rc, vot::AnimationComponent& ac, const vot::LoadOptions& options) -> void {
        const auto scene = mic.scene;
        uint32_t vertexOffset = 0, indexOffset = 0;
        extractCenter(mic, rc);
        assignBuffer(mic, vc);

        yic::logger->info("scene has {0} num meshes", scene->mNumMeshes);
        for(const auto aiMesh : std::span<aiMesh*>(scene->mMeshes, scene->mNumMeshes)){
            const auto aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
            extractVertex(aiMesh, vertexOffset, vc, options);
            extractBone(aiMesh, vertexOffset, vc, ac);
            extractIndex(aiMesh, vertexOffset, indexOffset, vc);

            vot::SubMesh subMesh{};
            subMesh.firstIndex = indexOffset;
            vertexOffset += aiMesh->mNumVertices;
            indexOffset += aiMesh->mNumFaces * 3;
            subMesh.indexCount = indexOffset - subMesh.firstIndex;

            buildAdjacencyIndex(vc.indices_pmr.data() + subMesh.firstIndex, subMesh.indexCount, vc.adjIndices_pmr.data() + subMesh.firstIndex * 2);
            extractDiffTex(mic, aiMat, subMesh, rc);
        }
        extractBoneNode(mic, ac);
        allocBuffer(mic, vc, rc);
        allocDSHandle(ac, rc);
    }

    auto AssimpLoader::extractAnim(const AssimpLoader::ImportContext &mic, vot::AnimationComponent& ac) -> void {
        const auto scene = mic.scene;
        if (!scene->HasAnimations()) {yic::logger->info("No animations in the model!"); return;};

        for(auto aiAnim : std::span<aiAnimation*>(scene->mAnimations, scene->mNumAnimations)){
            ac.animations.emplace_back(aiAnim->mName.data, std::move(std::make_shared<Animation>(aiAnim, ac)));
        }
    }

    auto AssimpLoader::extractBoneNode(const rs::AssimpLoader::ImportContext &ctx,
                                       vot::AnimationComponent &ac) -> void {
        for (auto& boneMats : ac.boneMats) {
            boneMats.resize(ac.boneCount, glm::mat4 (1.f));
        }
        if (ac.boneCount <= 0) {
            constexpr auto defaultMat = glm::mat4(1.f);
            ac.boneMatBuffer = yic::allocator->allocBufferStaging(sizeof (glm::mat4), &defaultMat, vk::BufferUsageFlagBits::eStorageBuffer, "bone matrices buf");
            return;
        }
        ac.boneMatBuffer = yic::allocator->allocBufferStaging(ac.boneCount * sizeof (glm::mat4), ac.boneMats[0].data(), vk::BufferUsageFlagBits::eStorageBuffer, "bone matrices buf");

        std::function<void(vot::BoneNode& boneNode, const aiNode* src)> readHierarchyData = [&](vot::BoneNode& boneNode, const aiNode* src){
            boneNode.name = src->mName.data;
            boneNode.transformation = miku::AssimpGLMConverter(src->mTransformation);
            boneNode.childrenCount = src->mNumChildren;

            for(auto i = 0u; i < src->mNumChildren; i++){
                vot::BoneNode node;
                readHierarchyData(node, src->mChildren[i]);
                boneNode.children.emplace_back(node);
            }
        };

        readHierarchyData(ac.boneNode, ctx.scene->mRootNode);
    }

    auto AssimpLoader::extractDiffTex(ImportContext& ctx, const aiMaterial* aiMat, const vot::SubMesh& subMesh, vot::RenderComponent& rc) -> void {
        auto findOrAdd = [&](const std::filesystem::path &pt) -> std::pair<uint32_t, bool> {
            auto [it, inserted] = ctx.pts.emplace(pt, 0);
            if (!inserted) {
                return {it->second, false};
            }
            uint32_t newIndex = ctx.pts.size() - 1;
            it->second = newIndex;
            return {newIndex, true};
        };
        aiString aiPt;
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &aiPt) == AI_SUCCESS) {
            if (const auto loadPt = fo::findFileInDirectory(std::filesystem::path(ctx.pt.c_str()).parent_path(), aiPt.C_Str()); loadPt.has_value()) {
                auto [index, alloc] = findOrAdd(loadPt.value());

                rc.subMeshes[index].emplace_back(subMesh);
                if (rc.diffuseTextures.size() <= index) rc.diffuseTextures.resize(index + 1);

                if (alloc) {
                    if (rc.diffuseTextures[index] == nullptr) {
                        //const auto utf8str = boost::locale::conv::utf_to_utf<char>(loadPt.value().u16string());
                        const auto utf8str = fo::utf16_to_utf_8(loadPt.value());
                        yic::logger->info(utf8str);
                        rc.diffuseTextures[index] = yic::allocator->loadTexture(utf8str.c_str());
                    }
                }
            }
        } else {
            rc.subMeshes[0].emplace_back(subMesh); // FIXME
            rc.diffuseTextures.push_back(defaultTex);
        }
    }

    auto AssimpLoader::extractIndex(const aiMesh *aiMesh, const uint32_t& vertexOffset, const uint32_t& indexOffset, vot::VertexDataComponent& vc) -> void {
        for (auto j = 0; j < aiMesh->mNumFaces; j++) {
            const auto &face = aiMesh->mFaces[j];
            vc.indices_pmr[j * 3 + indexOffset] = face.mIndices[0] + vertexOffset;
            vc.indices_pmr[j * 3 + indexOffset + 1] = face.mIndices[1] + vertexOffset;
            vc.indices_pmr[j * 3 + indexOffset + 2] = face.mIndices[2] + vertexOffset;
        }
    }


    auto AssimpLoader::extractBone(const aiMesh *aiMesh, const uint32_t& vertexOffset, vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void {
        for (auto j = 0; j < aiMesh->mNumBones; ++j) {
            const auto bone = aiMesh->mBones[j];

            auto boneId = -1;
            auto& boneMap = ac.boneMap;
            vot::string boneName{bone->mName.data};

            if (ac.boneMap.find(boneName) == boneMap.end()){
                boneId = ac.boneCount;
                const vot::BoneInfo boneInfo{
                        .id = ac.boneCount,
                        .offset = miku::AssimpGLMConverter(bone->mOffsetMatrix),
                };
                boneMap[boneName] = boneInfo;
                ac.boneCount++;
            } else {
                boneId = boneMap[boneName].id;
            }

            const auto weights = bone->mWeights;
            const auto numWeights = bone->mNumWeights;

            for (auto& pmr : vc.vertices_pmr) {

            for(auto wIndex = 0u; wIndex < numWeights; wIndex++){
                const auto vertId = weights[wIndex].mVertexId;
                const auto weight = weights[wIndex].mWeight;
                auto& vert = pmr[vertexOffset + vertId];

                for(auto x = 0; x < 4; ++x){
                    if (vert.boneIds[x] < 0){
                        vert.boneIds[x] = boneId;
                        vert.boneWeight[x] = weight;
                        break;
                    }
                }
            }
            }
        }
    }

    auto AssimpLoader::extractVertex(const aiMesh *aiMesh, const uint32_t &vertexOffset, vot::VertexDataComponent &vc, const vot::LoadOptions& options) -> void {
        for(auto j = 0; j < aiMesh->mNumVertices; ++j){
            vot::VertexT<vot::eAssimp> v{};

            if (aiMesh->HasPositions()) {
                auto &pos = aiMesh->mVertices[j];
                if (options.scale.has_value()) {
                    pos *= *options.scale;
                }
                std::memcpy(&v.pos, &pos, sizeof(glm::vec3));
            }
            if (aiMesh->HasNormals()) {
                auto &nor = aiMesh->mNormals[j];
                std::memcpy(&v.nor, &nor, sizeof(glm::vec3));
            }
            if (aiMesh->HasTextureCoords(0)) {
                auto &uv = aiMesh->mTextureCoords[0][j];
                std::memcpy(&v.uv, &uv, sizeof(glm::vec2));
            }

            for(auto k = 0; k < 4; k++){
                v.boneIds[k] = -1;
                v.boneWeight[k] = 0.f;
            }  // NOTE: pos nor uv boneId boneWeight

            for(auto& pmr : vc.vertices_pmr){
                pmr[j + vertexOffset] = v;
            }  // NOTE:
        }
    }

    auto AssimpLoader::extractCenter(const ImportContext& ctx, vot::RenderComponent &rc) -> void {
        rc.center = processMeshCenter(ctx.scene);
        rc.baseMat = glm::translate(glm::mat4 (1.f), -rc.center);
    }

    auto AssimpLoader::assignBuffer(const ImportContext& ctx, vot::VertexDataComponent& vc) -> void {
         for(auto& pmr : vc.vertices_pmr){
            pmr = std::pmr::vector<vot::VertexT<vot::eAssimp>>{&mVertexPool};
         }
        vc.indices_pmr = std::pmr::vector<uint32_t>{&mIndexPool};
        vc.adjIndices_pmr = std::pmr::vector<uint32_t>{&mAdjacencyIndexPool};

        uint32_t vertexCount = 0, indexCount = 0;
        for(auto i = 0; i < ctx.scene->mNumMeshes; ++i){
            const auto aiMesh = ctx.scene->mMeshes[i];
            vertexCount += aiMesh->mNumVertices;
            indexCount += aiMesh->mNumFaces * 3;
        }

        for(auto& pmr : vc.vertices_pmr){
            pmr.resize(vertexCount);
        }

        vc.indices_pmr.resize(indexCount);
        vc.adjIndices_pmr.resize(indexCount * 2);
    }

    auto AssimpLoader::processMeshCenter(const auto *scene) -> glm::vec3 {
        const aiScene *s = scene;

        auto min = _mm256_set1_ps(FLT_MAX);
        auto max = _mm256_set1_ps(FLT_MIN);

        for (uint32_t i = 0; i < s->mNumMeshes; i++) {
            const auto mesh = s->mMeshes[i];
            const auto ax = mesh->mAABB.mMax, in = mesh->mAABB.mMin;

            const auto meshMax = _mm256_set_ps(0, 0, 0, 0, ax.z, ax.y, ax.x, 0);
            const auto meshMin = _mm256_set_ps(0, 0, 0, 0, in.z, in.y, in.x, 0);

            min = _mm256_min_ps(min, meshMin);
            max = _mm256_max_ps(max, meshMax);

        }

        float finalMin[8], finalMax[8];
        _mm256_storeu_ps(finalMin, min);
        _mm256_storeu_ps(finalMax, max);

        auto center = glm::vec3{
                (finalMin[1] + finalMax[1]) / 2.f,
                (finalMin[2] + finalMax[2]) / 2.f,
                (finalMin[3] + finalMax[3]) / 2.f,
        };

        yic::logger->info("minX:{0}, maxX:{1}", finalMin[1], finalMax[1]);
        yic::logger->info("minY:{0}, maxY:{1}", finalMin[2], finalMax[2]);
        yic::logger->info("minZ:{0}, maxZ:{1}", finalMin[3], finalMax[3]);
        yic::logger->info("center{0}, {1}, {2}", center.x, center.y, center.z);

        return center;
    }

    auto AssimpLoader::buildAdjacencyIndex(const uint32_t *indexChunkFirst, const uint32_t offset,
                                           uint32_t *adjIndexChunk) -> void {
        const size_t triCount = offset / 3;
        vot::unordered_map<uint64_t, EdgeEntry> edgeMap;
        edgeMap.reserve(triCount * 3);

        for(auto t = 0; t < triCount; ++t){
            const auto v0 = indexChunkFirst[3 * t + 0];
            const auto v1 = indexChunkFirst[3 * t + 1];
            const auto v2 = indexChunkFirst[3 * t + 2];
            const uint32_t verts[3] = {v0, v1, v2};

            for(auto e = 0; e < 3; ++e){
                const auto a = verts[e];
                const auto b = verts[(e + 1) % 3];
                const auto opp = verts[(e + 2) % 3];
                uint64_t key = makeEdgeKey(a, b);
                auto& ent = edgeMap[key];

                if (ent.opp0 == UINT32_MAX) ent.opp0 = opp;
                else if (ent.opp1 == UINT32_MAX) ent.opp1 = opp;
            }
        }

        for(uint32_t t = 0; t < triCount; ++t){
            const auto v0 = indexChunkFirst[3 * t + 0];
            const auto v1 = indexChunkFirst[3 * t + 1];
            const auto v2 = indexChunkFirst[3 * t + 2];
            const uint32_t verts[3] = {v0, v1, v2};

            for(auto e = 0; e < 3; ++e){
                const auto a = verts[e];
                const auto b = verts[(e + 1) % 3];
                uint64_t key = makeEdgeKey(a, b);
                const auto& ent = edgeMap[key];

                const auto adjOutIndex = static_cast<uint32_t>(t * 6 + e * 2);
                adjIndexChunk[adjOutIndex + 0] = a;

                auto adj = a;
                if (ent.opp0 != UINT32_MAX && ent.opp0 != a) adj = ent.opp0;
                else if(ent.opp1 != UINT32_MAX && ent.opp1 != a) adj = ent.opp1;

                adjIndexChunk[adjOutIndex + 1] = adj;
            }
        }
    }

    auto AssimpLoader::allocBuffer(const ImportContext& mic, const vot::VertexDataComponent& vc, vot::RenderComponent& rc) -> void {
        constexpr auto usage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        auto annotate = [&](const vot::string &id) { return " model: " + mic.fileName + " " + id + " buf"; };

        for(auto& vb : rc.vertexBuffer){
            vb = yic::allocator->allocBufferStaging(vc.vertices_pmr[0].size() * sizeof(vot::VertexT<vot::eAssimp>),
                                                             vc.vertices_pmr[0].data(),
                                                             usage | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
                                                             annotate("vert"));
        }
        rc.indexBuffer = yic::allocator->allocBufferStaging(vc.indices_pmr.size() * sizeof(uint32_t),
                                                            vc.indices_pmr.data(),
                                                            usage | vk::BufferUsageFlagBits::eIndexBuffer,
                                                            annotate("index"));
        rc.adjIndexBuffer = yic::allocator->allocBufferStaging(vc.adjIndices_pmr.size() * sizeof(uint32_t),
                                                               vc.adjIndices_pmr.data(),
                                                               usage | vk::BufferUsageFlagBits::eIndexBuffer,
                                                               annotate("adj index"));
    }

    auto AssimpLoader::allocDSHandle(const vot::AnimationComponent& ac, vot::RenderComponent& rc) -> void {
        rc.dsHandle = yic::desSystem->allocUpdateDescriptorSets([&]{
           vot::DescriptorLayout2 layout{};


           for(const auto& img : rc.diffuseTextures){
               layout.emplace(vot::DescriptorLayout2::_1d{
                   img->imageInfo(),
                   ac.boneMatBuffer->bufferInfo(),
                   rc.vertexBuffer[vot::VertexDataComponent::eTPose]->bufferInfo(),
                   rc.vertexBuffer[vot::VertexDataComponent::eAnim]->bufferInfo(),
               });
           }

           return layout;
       }, yic::renderLibrary->GP_Basic_Assimp);
    }

    auto AssimpLoader::makeEdgeKey(uint32_t a, uint32_t b) -> uint64_t {
        const uint32_t lo = std::min(a, b), hi = std::max(a, b);
        return (static_cast<uint64_t>(lo) << 32) | hi;
    }

    auto AssimpLoader::resolvingPath(const vot::string& pt) -> std::shared_ptr<void> {
        auto importer = std::make_shared<Assimp::Importer>();
        auto directionPt = std::filesystem::path(pt.c_str()).parent_path();
        const auto lastDotPos = pt.find_last_of('.');
        auto ext = (lastDotPos != std::string::npos) ? pt.substr(lastDotPos + 1) : "";

        const auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate | aiProcess_LimitBoneWeights | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
//        auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_LimitBoneWeights | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            throw std::runtime_error("assimp load model error: " + std::string (importer->GetErrorString()));

        return {const_cast<aiScene*>(scene), [importer](void* ptr){}};
    }

} // rs


//    auto ModelManager::transformVerticesCUDA(const glm::vec3& center, aiMesh* aiMesh) -> void {
//        float3 *vertices;
//        cudaMalloc(&vertices, aiMesh->mNumVertices * sizeof(float3));
//        cudaMemcpy(vertices, aiMesh->mVertices, aiMesh->mNumVertices * sizeof(float3), cudaMemcpyHostToDevice);
//        float3 c;
//        c.x = center.x;
//        c.y = center.y;
//        c.z = center.z;
//        gpu::transformVerticesCUDA(vertices, c, aiMesh->mNumVertices);
//        cudaMemcpy(aiMesh->mVertices, vertices, aiMesh->mNumVertices * sizeof(float3), cudaMemcpyDeviceToHost);
//        cudaFree(vertices);
//    }