//
// Created by lenovo on 6/15/2025.
//

#include "AssimpLoader.h"

#include "Utils/FileOperation.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "RHI/Command.h"
#include "RHI/DescriptorSystem.h"
#include "ECS/System/RenderLibrary.h"

namespace rs {

    AssimpLoader::AssimpLoader() {
        ct = yic::systemHub.val<ev::pVkSetupContext>();
    }

    AssimpLoader::~AssimpLoader() = default;

    auto AssimpLoader::Load(const vot::string &pt, vot::BasicInfoComponent &basicInfoComponent,
                                 vot::VertexDataComponent &vertexDataComponent,
                                 vot::RenderComponent &renderComponent, vot::AnimationComponent &animationComponent) -> vot::string {
        auto ctx = importScene(pt);

        extractMesh(ctx, vertexDataComponent, renderComponent, animationComponent);
        extractAnim(ctx, animationComponent);

        return basicInfoComponent.name = ctx.fileName;
    }

    auto AssimpLoader::importScene(const vot::string &pt) -> ImportContext {
        auto importer = std::make_shared<Assimp::Importer>();
        auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_LimitBoneWeights | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            throw std::runtime_error("assimp load model error: " + std::string (importer->GetErrorString()));
        std::filesystem::path p(pt);

        return {.importer = std::move(importer), .scene = scene,
                .fileName = p.stem().string().data(), .pt = pt, };
    }

    auto AssimpLoader::extractMesh(ImportContext& ctx, vot::VertexDataComponent& vc, vot::RenderComponent& rc, vot::AnimationComponent& ac) -> void {
        auto scene = ctx.scene;
        uint32_t vertexOffset = 0, indexOffset = 0;
        extractCenter(ctx, rc);
        assignBuffer(ctx, vc);

        yic::logger->info("scene has {0} num meshes", scene->mNumMeshes);
        for(auto aiMesh : std::span<aiMesh*>(scene->mMeshes, scene->mNumMeshes)){
            auto aiMat = scene->mMaterials[aiMesh->mMaterialIndex];
            extractVertex(aiMesh, vertexOffset, vc);
            extractBone(aiMesh, vertexOffset, vc, ac);
            extractIndex(aiMesh, vertexOffset, indexOffset, vc);

            vot::SubMesh subMesh{};
            subMesh.firstIndex = indexOffset;
            vertexOffset += aiMesh->mNumVertices;
            indexOffset += aiMesh->mNumFaces * 3;
            subMesh.indexCount = indexOffset - subMesh.firstIndex;

            buildAdjacencyIndex(vc.indices_pmr.data() + subMesh.firstIndex, subMesh.indexCount, vc.adjIndices_pmr.data() + subMesh.firstIndex * 2);
            extractDiffTex(ctx, aiMat, subMesh, rc);

            auto usage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
            auto annotate = [&](const vot::string &id) { return " model: " + ctx.fileName + " " + id + " buf"; };

            rc.vertexBuffer = yic::allocator->allocBufferStaging(vc.vertices_pmr.size() * sizeof(vot::Vertex),
                                                                 vc.vertices_pmr.data(),
                                                                 usage | vk::BufferUsageFlagBits::eVertexBuffer,
                                                                 annotate("vert"));
            rc.indexBuffer = yic::allocator->allocBufferStaging(vc.indices_pmr.size() * sizeof(uint32_t),
                                                                vc.indices_pmr.data(),
                                                                usage | vk::BufferUsageFlagBits::eIndexBuffer,
                                                                annotate("index"));
            rc.adjIndexBuffer = yic::allocator->allocBufferStaging(vc.adjIndices_pmr.size() * sizeof(uint32_t),
                                                                   vc.adjIndices_pmr.data(),
                                                                   usage | vk::BufferUsageFlagBits::eIndexBuffer,
                                                                   annotate("adj index"));
        }

        extractBoneNode(ctx, ac);

        rc.dsHandle = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};


            for(const auto& img : rc.diffuseTextures){
                layout.emplace(vot::DescriptorLayout2::_1d{
                        img->imageInfo(),
                        ac.boneMatBuffer->bufferInfo(),
                });
            }

            return layout;
        }, yic::renderLibrary->GP_Basic);
    }

    auto AssimpLoader::extractAnim(AssimpLoader::ImportContext &mic, vot::AnimationComponent& ac) -> void {
        auto scene = mic.scene;
        if (!scene->HasAnimations()) {yic::logger->info("No animations in the model!"); return;};

        for(auto aiAnim : std::span<aiAnimation*>(scene->mAnimations, scene->mNumAnimations)){
            ac.animations.emplace_back(aiAnim->mName.data, std::move(std::make_shared<Animation>(aiAnim, ac)));
        }
    }

    auto AssimpLoader::extractBoneNode(const rs::AssimpLoader::ImportContext &ctx,
                                       vot::AnimationComponent &ac) -> void {
        ac.boneMats.resize(ac.boneCount, glm::mat4 (1.f));
        ac.boneMatBuffer = yic::allocator->allocBufferStaging(ac.boneMats.size() * sizeof (glm::mat4), ac.boneMats.data(), vk::BufferUsageFlagBits::eStorageBuffer, "bone matrices buf");

        std::function<void(vot::BoneNode& boneNode, const aiNode* src)> readHierarchyData = [&](vot::BoneNode& boneNode, const aiNode* src){
            boneNode.name = src->mName.data;
            boneNode.transformation = miku::AssimpGLMConverter(src->mTransformation);
            boneNode.childrenCount = src->mNumChildren;

//            yic::logger->info("Bone Node Name: {}, Transformation: {}", boneNode.name, glm::to_string(boneNode.transformation));

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
            } else {
                uint32_t newIndex = ctx.pts.size() - 1;
                it->second = newIndex;
                return {newIndex, true};
            }
        };
        aiString aiPt;
        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &aiPt) == AI_SUCCESS) {
            auto loadPt = fo::findFileInDirectory(std::filesystem::path(ctx.pt.c_str()).parent_path(), aiPt.C_Str());
            if (loadPt.has_value()) {
                auto [index, alloc] = findOrAdd(loadPt.value());

                rc.subMeshes[index].emplace_back(subMesh);
                if (rc.diffuseTextures.size() <= index) rc.diffuseTextures.resize(index + 1);

                if (alloc) {
                    if (rc.diffuseTextures[index] == nullptr) {
                        auto utf8str = boost::locale::conv::utf_to_utf<char>(loadPt.value().u16string());
                        yic::logger->info(utf8str);
                        rc.diffuseTextures[index] = yic::allocator->loadTexture(utf8str.c_str());
                    }
                }
            }
        }
    }

    auto AssimpLoader::extractIndex(const aiMesh *aiMesh, const uint32_t& vertexOffset, const uint32_t& indexOffset, vot::VertexDataComponent& vc) -> void {
        for (auto j = 0; j < aiMesh->mNumFaces; j++) {
            auto &face = aiMesh->mFaces[j];
            vc.indices_pmr[j * 3 + indexOffset] = face.mIndices[0] + vertexOffset;
            vc.indices_pmr[j * 3 + indexOffset + 1] = face.mIndices[1] + vertexOffset;
            vc.indices_pmr[j * 3 + indexOffset + 2] = face.mIndices[2] + vertexOffset;
        }
    }


    auto AssimpLoader::extractBone(const aiMesh *aiMesh, const uint32_t& vertexOffset, vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void {
        for (auto j = 0; j < aiMesh->mNumBones; ++j) {
            auto bone = aiMesh->mBones[j];

            auto boneId = -1;
            auto& boneMap = ac.boneMap;
            vot::string boneName{bone->mName.data};

            if (ac.boneMap.find(boneName) == boneMap.end()){
                boneId = ac.boneCount;
                vot::BoneInfo boneInfo{
                        .id = ac.boneCount,
                        .offset = miku::AssimpGLMConverter(bone->mOffsetMatrix),
                };
                boneMap[boneName] = boneInfo;
                ac.boneCount++;
            } else {
                boneId = boneMap[boneName].id;
            }

            auto weights = bone->mWeights;
            auto numWeights = bone->mNumWeights;

            for(auto wIndex = 0u; wIndex < numWeights; wIndex++){
                auto vertId = weights[wIndex].mVertexId;
                auto weight = weights[wIndex].mWeight;
                auto& vert = vc.vertices_pmr[vertexOffset + vertId];

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

    auto AssimpLoader::extractVertex(const aiMesh *aiMesh, const uint32_t &vertexOffset, vot::VertexDataComponent &vc) -> void {
        for(auto j = 0; j < aiMesh->mNumVertices; ++j){
            vot::Vertex v{};

            if (aiMesh->HasPositions()) {
                auto &pos = aiMesh->mVertices[j];
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
            }

            vc.vertices_pmr[j + vertexOffset] = v;
        }
    }

    auto AssimpLoader::extractCenter(const ImportContext& ctx, vot::RenderComponent &rc) -> void {
        rc.center = processMeshCenter(ctx.scene);
        rc.baseMat = glm::translate(glm::mat4 (1.f), -rc.center);
    }

    auto AssimpLoader::assignBuffer(const ImportContext& ctx, vot::VertexDataComponent& vc) -> void {
        vc.vertices_pmr = std::pmr::vector<vot::Vertex>{&mVertexPool};
        vc.indices_pmr = std::pmr::vector<uint32_t>{&mIndexPool};
        vc.adjIndices_pmr = std::pmr::vector<uint32_t>{&mAdjacencyIndexPool};

        uint32_t vertexCount = 0, indexCount = 0;
        for(auto i = 0; i < ctx.scene->mNumMeshes; ++i){
            auto aiMesh = ctx.scene->mMeshes[i];
            vertexCount += aiMesh->mNumVertices;
            indexCount += aiMesh->mNumFaces * 3;
        }

        vc.vertices_pmr.resize(vertexCount);
        vc.indices_pmr.resize(indexCount);
        vc.adjIndices_pmr.resize(indexCount * 2);
    }

    auto AssimpLoader::processMeshCenter(const auto *scene) -> glm::vec3 {
        const aiScene *s = scene;

        auto min = _mm256_set1_ps(FLT_MAX);
        auto max = _mm256_set1_ps(FLT_MIN);

        for (uint32_t i = 0; i < s->mNumMeshes; i++) {
            auto mesh = s->mMeshes[i];
            auto ax = mesh->mAABB.mMax, in = mesh->mAABB.mMin;

            auto meshMax = _mm256_set_ps(0, 0, 0, 0, ax.z, ax.y, ax.x, 0);
            auto meshMin = _mm256_set_ps(0, 0, 0, 0, in.z, in.y, in.x, 0);

            min = _mm256_min_ps(min, meshMin);
            max = _mm256_max_ps(max, meshMax);

//            model.mesh.subMeshes[i].aabb = {{ax.x, ax.y, ax.z},
//                                            {in.x, in.y, in.z}};
        }

        float finalMin[8], finalMax[8];
        _mm256_storeu_ps(finalMin, min);
        _mm256_storeu_ps(finalMax, max);

//        auto aabb = vot::AABB{
//                {finalMax[1], finalMax[2], finalMax[3]},
//                {finalMin[1], finalMin[2], finalMin[3]}
//        };

        auto center = glm::vec3{
                (finalMin[1] + finalMax[1]) / 2.f,
                (finalMin[2] + finalMax[2]) / 2.f,
                (finalMin[3] + finalMax[3]) / 2.f,
        };

        return center;
    }

    auto AssimpLoader::buildAdjacencyIndex(const uint32_t *indexChunkFirst, const uint32_t offset,
                                           uint32_t *adjIndexChunk) -> void {
        size_t triCount = offset / 3;
        vot::unordered_map<uint64_t, EdgeEntry> edgeMap;
        edgeMap.reserve(triCount * 3);

        for(auto t = 0; t < triCount; ++t){
            auto v0 = indexChunkFirst[3 * t + 0];
            auto v1 = indexChunkFirst[3 * t + 1];
            auto v2 = indexChunkFirst[3 * t + 2];
            uint32_t verts[3] = {v0, v1, v2};

            for(auto e = 0; e < 3; ++e){
                auto a = verts[e];
                auto b = verts[(e + 1) % 3];
                auto opp = verts[(e + 2) % 3];
                uint64_t key = makeEdgeKey(a, b);
                auto& ent = edgeMap[key];

                if (ent.opp0 == UINT32_MAX) ent.opp0 = opp;
                else if (ent.opp1 == UINT32_MAX) ent.opp1 = opp;
            }
        }

        for(uint32_t t = 0; t < triCount; ++t){
            auto v0 = indexChunkFirst[3 * t + 0];
            auto v1 = indexChunkFirst[3 * t + 1];
            auto v2 = indexChunkFirst[3 * t + 2];
            uint32_t verts[3] = {v0, v1, v2};

            for(auto e = 0; e < 3; ++e){
                auto a = verts[e];
                auto b = verts[(e + 1) % 3];
                uint64_t key = makeEdgeKey(a, b);
                auto& ent = edgeMap[key];

                auto adjOutIndex = static_cast<uint32_t>(t * 6 + e * 2);
                adjIndexChunk[adjOutIndex + 0] = a;

                auto adj = a;
                if (ent.opp0 != UINT32_MAX && ent.opp0 != a) adj = ent.opp0;
                else if(ent.opp1 != UINT32_MAX && ent.opp1 != a) adj = ent.opp1;

                adjIndexChunk[adjOutIndex + 1] = adj;
            }
        }
    }

    auto AssimpLoader::makeEdgeKey(uint32_t a, uint32_t b) -> uint64_t {
        uint32_t lo = std::min(a, b), hi = std::max(a, b);
        return (uint64_t (lo) << 32) | hi;
    }

    auto AssimpLoader::resolvingPath(const vot::string& pt) -> std::shared_ptr<void> {
        auto importer = std::make_shared<Assimp::Importer>();
        auto directionPt = std::filesystem::path(pt.c_str()).parent_path();
        auto lastDotPos = pt.find_last_of('.');
        auto ext = (lastDotPos != std::string::npos) ? pt.substr(lastDotPos + 1) : "";

//        auto scene = importer->ReadFile(pt.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_GenBoundingBoxes | aiProcess_GenUVCoords);
//        auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_LimitBoneWeights | aiProcess_GenBoundingBoxes);
        auto scene = importer->ReadFile(pt.c_str(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_LimitBoneWeights | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
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