//
// Created by lenovo on 7/21/2024.
//

#include "ModelLoader.h"

#include "Engine/RHI/Allocator.h"

namespace sc {

    auto ModelLoader::Load(const std::string &path) -> Model {
      Assimp::Importer importer;
      auto dirPt = std::filesystem::path(path).parent_path();
      auto lastDotPos = path.find_last_of('.');
      auto ext = (lastDotPos != std::string::npos) ? path.substr(lastDotPos + 1) : "";

        auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals |
                                             aiProcess_GenBoundingBoxes | aiProcess_GenUVCoords);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            throw std::runtime_error("assimp load model error: " + std::string (importer.GetErrorString()));
        }

        return process(scene, dirPt);
    };

    auto ModelLoader::process(const aiScene *scene, const std::filesystem::path &modelDirPath) -> Model {
        Model model;
        model.info.id = scene->mRootNode->mName.C_Str();
        auto& mesh = model.mesh;
        auto& texs = model.texs;
        auto& subMeshes = mesh.subMeshes;
        subMeshes.resize(scene->mNumMeshes);

        auto center = processMeshCenter(scene, model);
        size_t vertOffset = 0;

        std::unordered_map<pt, uint32_t> paths;
        for (auto i = 0; i < scene->mNumMeshes; i++) {
            auto aiMesh = scene->mMeshes[i];
            auto aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
            auto& subMesh = subMeshes[i];

            for (auto j = 0; j < aiMesh->mNumVertices; j++) {
                Vertex vertex{};

                if (aiMesh->HasPositions()) {
                    auto &pos = aiMesh->mVertices[j];
                    vertex.pos = {pos.x - center.x, pos.y - center.y, pos.z - center.z};
                }
                if (aiMesh->HasNormals()) {
                    auto &nor = aiMesh->mNormals[j];
                    std::memcpy(&vertex.nor, &nor, sizeof(glm::vec3));
                }
                if (aiMesh->HasTextureCoords(0)) {
                    auto &uv = aiMesh->mTextureCoords[0][j];
                    std::memcpy(&vertex.uv, &uv, sizeof(glm::vec2));
                }

                mesh.vertices.emplace_back(vertex);
            }

            subMesh.firstIndex = mesh.indices.size();
            for (auto j = 0; j < aiMesh->mNumFaces; j++) {
                auto &face = aiMesh->mFaces[j];
                for (auto k = 0; k < face.mNumIndices; k++) {
                    mesh.indices.emplace_back(face.mIndices[k] + vertOffset);
                }
            }
            vertOffset = mesh.vertices.size();
            subMesh.indexCount = mesh.indices.size() - subMesh.firstIndex;

            bool texAlloc = false;
            auto findOrAdd = [&](const pt &p) -> uint32_t {
                std::string pathStr = p.string();
                auto [iter, inserted] = paths.emplace(pathStr, 0);
                if (!inserted) {
                    return iter->second;
                } else {
                    uint32_t newIndex = paths.size() - 1;
                    iter->second = newIndex;
                    texAlloc = true;
                    return newIndex;
                }
            };
            aiString path;
            if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
                pt texPath(path.C_Str());

                auto loadPt = fo::findFileInDirectory(modelDirPath, texPath.filename());
                auto index = findOrAdd(loadPt);
                subMesh.texIndex = index;
                if (texs.diffTexs.size() <= index) {
                    texs.diffTexs.resize(index + 1);
                }
                if (texAlloc) {
                    if (texs.diffTexs[index] == nullptr) {
                        texs.diffTexs[index] = yic::Allocator::allocImg(loadPt.string());
                    }
                }
            }
        }

        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        mesh.vertBuf = yic::Allocator::allocBufStaging(mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), vk::BufferUsageFlagBits::eVertexBuffer | usage);
        mesh.indexBuf = yic::Allocator::allocBufStaging(mesh.indices.size() * sizeof(uint32_t), mesh.indices.data(), vk::BufferUsageFlagBits::eIndexBuffer | usage);

        return model;
    }

    auto ModelLoader::processMeshCenter(const aiScene *scene, sc::Model &model) -> aiVector3D {
        auto min = _mm256_set1_ps(FLT_MAX);
        auto max = _mm256_set1_ps(FLT_MIN);

        for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
            auto mesh = scene->mMeshes[i];
            auto ax = mesh->mAABB.mMax, in = mesh->mAABB.mMin;

            auto meshMax = _mm256_set_ps(0, 0, 0, 0, ax.z, ax.y, ax.x, 0);
            auto meshMin = _mm256_set_ps(0, 0, 0, 0, in.z, in.y, in.x, 0);

            min = _mm256_min_ps(min, meshMin);
            max = _mm256_max_ps(max, meshMax);

            model.mesh.subMeshes[i].aabb = {{ax.x, ax.y, ax.z},
                                    {in.x, in.y, in.z}};
        }

        float finalMin[8], finalMax[8];
        _mm256_storeu_ps(finalMin, min);
        _mm256_storeu_ps(finalMax, max);

        model.mesh.aabb = {
                {finalMax[4], finalMax[3], finalMax[2]},
                {finalMin[4], finalMin[3], finalMin[2]}
        };

        return {
                (finalMin[1] + finalMax[1]) / 2.f,
                (finalMin[2] + finalMax[2]) / 2.f,
                (finalMin[3] + finalMax[3]) / 2.f,
        };
    }

} // sc