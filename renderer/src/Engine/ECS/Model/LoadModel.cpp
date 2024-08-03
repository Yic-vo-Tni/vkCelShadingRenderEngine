//
// Created by lenovo on 7/21/2024.
//

#include "LoadModel.h"

#include "Engine/RHI/Allocator.h"

namespace sc {

    auto ModelLoader::Load(const std::string &path, PipelineDesSetLayout& setLayout) -> Model {
        Assimp::Importer importer;
        auto modelDirectoryPath = std::filesystem::path(path).parent_path();
        auto lastDotPos = path.find_last_of('.');
        auto extension = (lastDotPos != std::string::npos) ? path.substr(lastDotPos + 1) : "";

        auto scene = importer.ReadFile(path,
                                       aiProcess_Triangulate | aiProcess_GenSmoothNormals
                                       | aiProcess_FlipUVs | aiProcess_FlipWindingOrder);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            vkError("assimp load model error: " + std::string(importer.GetErrorString()));
        }

        return process(scene, modelDirectoryPath, setLayout);
    }

    auto ModelLoader::process(const aiScene *scene, const std::filesystem::path &modelDirPath, PipelineDesSetLayout& setLayout) -> Model {
        Model model;
        auto& meshes = model.meshes;
        meshes.resize(scene->mNumMeshes);

        oneapi::tbb::concurrent_unordered_map<pt, uint32_t> paths;

        auto center = processMeshCenter(scene, model);

        oneapi::tbb::global_control control(oneapi::tbb::global_control::max_allowed_parallelism, 8);

        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<uint32_t>(0, scene->mNumMeshes),
                [&](const oneapi::tbb::blocked_range<uint32_t>& range){
            for(auto i = range.begin(); i != range.end(); i++){
                Mesh mesh;
                auto aiMesh = scene->mMeshes[i];
                auto aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];

                processMesh(aiMesh, mesh, center);
                processMaterial(aiMaterial, model, mesh, paths, modelDirPath);

                meshes[i] = mesh;
            }
        });

        auto id = scene->mRootNode->mName.C_Str();
        model.id = id;
        model.descriptor = std::make_shared<yic::Descriptor>(id, setLayout);
        std::vector<std::shared_ptr<yic::vkImage>> texs(model.diffTexs.begin(), model.diffTexs.end());
        model.descriptor->updateDesSet(model.diffTexs.size(), {yic::ImgInfo{texs}});

        return model;
    }

    auto ModelLoader::processMesh(const aiMesh *aiMesh, Mesh& mesh, const aiVector3D& center) -> void {
        mesh.vertices.reserve(aiMesh->mNumVertices);

        for (uint32_t j = 0; j < aiMesh->mNumVertices; j++) {
            Vertex vertex{};

            if (aiMesh->HasPositions()) {
                auto pos = aiMesh->mVertices[j];
                vertex.pos = {pos.x - center.x, pos.y - center.y, pos.z - center.z};
            }
            if (aiMesh->HasNormals()) {
                auto nor = aiMesh->mNormals[j];
                vertex.nor = {nor.x, nor.y, nor.z};
            }
            if (aiMesh->HasTextureCoords(0)) {
                auto uv = aiMesh->mTextureCoords[0][j];
                vertex.uv = {uv.x, uv.y};
            }

            mesh.vertices.emplace_back(vertex);
        }

        mesh.indices.reserve(aiMesh->mNumFaces);
        for (uint32_t j = 0; j < aiMesh->mNumFaces; j++) {
            auto face = aiMesh->mFaces[j];
            for (uint32_t k = 0; k < face.mNumIndices; k++) {
                mesh.indices.emplace_back(face.mIndices[k]);
            }
        }

        mesh.vertBuf = yic::Allocator::allocBufStaging(sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), vk::BufferUsageFlagBits::eVertexBuffer, yic::Allocator::MemoryUsage::eGpuOnly, IdGenerator::uniqueId());
        mesh.indexBuf = yic::Allocator::allocBufStaging(sizeof(uint32_t) * mesh.indices.size(), mesh.indices.data(), vk::BufferUsageFlagBits::eIndexBuffer, yic::Allocator::MemoryUsage::eGpuOnly, IdGenerator::uniqueId());
    }

    auto ModelLoader::processMaterial(aiMaterial *aiMaterial, Model& model, Mesh& mesh, oneapi::tbb::concurrent_unordered_map<pt, uint32_t>& paths, const std::filesystem::path& modelDirPath) -> void {
        bool texAlloc = false;
        auto findOrAdd = [&](const pt& p) -> uint32_t {
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
        if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
            pt texPath(path.C_Str());

            auto loadPt = fo::findFileInDirectory(modelDirPath, texPath.filename());
            auto index = findOrAdd(loadPt);
            mesh.texIndex = index;
            if (model.diffTexs.size() <= index){
                //model.diffTexs.grow_by(index + 1 - model.diffTexs.size());
                model.diffTexs.resize(index + 1);
            }
            if (texAlloc){
                if (model.diffTexs[index] == nullptr) {
                    model.diffTexs[index] = yic::Allocator::allocImg(loadPt.string());
                }
            }

        }
    }

    auto ModelLoader::processMeshCenter(const aiScene* scene, Model& model) -> aiVector3D {
        aiVector3D min{FLT_MAX}, max{FLT_MIN};

        for(uint32_t i = 0; i < scene->mNumMeshes; i++){
            auto mesh = scene->mMeshes[i];
            auto ax = mesh->mAABB.mMax, in = mesh->mAABB.mMin;
            model.meshes[i].aabb = {
                    .max = {ax.x, ax.y, ax.z},
                    .min = {in.x, in.y, in.z},
            };
            min.x = std::min(min.x, in.x);
            min.y = std::min(min.y, in.y);
            min.z = std::min(min.z, in.z);
            max.x = std::max(max.x, ax.x);
            max.y = std::max(max.y, ax.y);
            max.z = std::max(max.z, ax.z);
        }

        model.aabb = {
                .max = {max.x, max.y, max.z},
                .min = {min.x, min.y, min.z},
        };

        return aiVector3D {
                (min.x + max.x) / 2.f,
                (min.y + max.y) / 2.f,
                (min.z + max.z) / 2.f,
        };

//        ///
//        oneapi::tbb::combinable<aiVector3D> min_com{FLT_MAX};
//        oneapi::tbb::combinable<aiVector3D> max_com{FLT_MIN};
//
//        auto& min_ = min_com.local();
//        auto& max_ = max_com.local();
//
//        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<uint32_t>(0, scene->mNumMeshes),
//                [&](const oneapi::tbb::blocked_range<uint32_t>& range){
//            for(uint32_t i = range.begin(); i != range.end(); i++){
//                const auto& mesh = scene->mMeshes[i];
//                const auto& ax = mesh->mAABB.mMax;
//                const auto& in = mesh->mAABB.mMin;
//
//                min_.x = std::min(min_.x, in.x);
//                min_.y = std::min(min_.y, in.y);
//                min_.z = std::min(min_.z, in.z);
//                max_.x = std::max(max_.x, ax.x);
//                max_.y = std::max(max_.y, ax.y);
//                max_.z = std::max(max_.z, ax.z);
//            }
//        });
//
//        auto finalMin = min_com.combine([](aiVector3D a, aiVector3D b){
//           return aiVector3D{std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
//        });
//        auto finalMax = max_com.combine([](aiVector3D a, aiVector3D b){
//            return aiVector3D{std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
//        });
//
//        return aiVector3D {
//                (finalMin.x + finalMax.x) / 2.f,
//                (finalMin.y + finalMax.y) / 2.f,
//                (finalMin.z + finalMax.z) / 2.f,
//        };
    }




} // sc