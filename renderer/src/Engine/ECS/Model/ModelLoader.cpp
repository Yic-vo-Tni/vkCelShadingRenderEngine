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

//        auto& as = model.as;
//        auto rtInput = yic::Allocator::modelToGeometryKHR(model);
//        as.blas = yic::Allocator::buildBLAS(rtInput);
//
//        vk::TransformMatrixKHR identityMatrix;
//        glm::mat4 matrix{1.f};
//        auto temp = glm::transpose(matrix);
//        memcpy(&identityMatrix, &temp, sizeof(vk::TransformMatrixKHR));
//
//        auto tfMatrix = yic::Allocator::glmMatToVkTransformMatrix();
//        std::vector<vk::AccelerationStructureInstanceKHR> asInsts;
//        vk::AccelerationStructureInstanceKHR asInst{
//                identityMatrix, 0, 0xFF, 0,
//                vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable, yic::Allocator::getAccelDevAddr(as.blas)
//        };
//        asInsts.emplace_back(asInst);
//        *as.tlas = yic::Allocator::buildTLAS(asInsts);

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

//    auto ModelLoader::Load(const std::string &path, PipelineDesSetLayout& setLayout) -> Model::Generic {
//        Assimp::Importer importer;
//        auto modelDirectoryPath = std::filesystem::path(path).parent_path();
//        auto lastDotPos = path.find_last_of('.');
//        auto extension = (lastDotPos != std::string::npos) ? path.substr(lastDotPos + 1) : "";
//
////        auto scene = importer.ReadFile(path,
////                                       aiProcess_Triangulate | aiProcess_GenSmoothNormals
////                                       | aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_GenBoundingBoxes);
//        auto scene = importer.ReadFile(path,
//                                       aiProcess_Triangulate | aiProcess_GenSmoothNormals
//                                       | aiProcess_FlipUVs | aiProcess_GenBoundingBoxes);
//        vkInfo("import model");
//
//        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
//            vkError("assimp load model error: " + std::string(importer.GetErrorString()));
//        }
//
//        return process(scene, modelDirectoryPath, setLayout);
//    }
//
//    auto ModelLoader::process(const aiScene *scene, const std::filesystem::path &modelDirPath, PipelineDesSetLayout& setLayout) -> Model::Generic {
//        Model::Generic model;
//        auto& meshes = model.meshes;
//        meshes.resize(scene->mNumMeshes);
//
//        oneapi::tbb::concurrent_unordered_map<pt, uint32_t> paths;
//
//        auto center = processMeshCenter(scene, model);
//
//        oneapi::tbb::global_control control(oneapi::tbb::global_control::max_allowed_parallelism, 8);
//
//        vkInfo("process model");
//        try {
//            oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<uint32_t>(0, scene->mNumMeshes),
//                                      [&](const oneapi::tbb::blocked_range<uint32_t> &range) {
//                                          std::stringstream localLog;
//                                          for (auto i = range.begin(); i != range.end(); i++) {
//                                              try {
//                                                  Mesh mesh;
//                                                  auto aiMesh = scene->mMeshes[i];
//                                                  auto aiMaterial = scene->mMaterials[aiMesh->mMaterialIndex];
//
//                                                  if_debug localLog << "start : " << i ;
//
//                                                  processMesh(aiMesh, mesh, center);
//                                                  processMaterial(aiMaterial, model, mesh, paths, modelDirPath);
//
//                                                  if_debug localLog << "end : " << i << "\n";
//
//                                                  meshes[i] = mesh;
//                                              } catch (const std::exception &e) {
//                                                  localLog << "Exception while processing mesh at index " << i << ": "
//                                                           << e.what() << "\n";
//                                              }
//                                          }
//                                          std::cerr << localLog.str();
//                                      });
//        } catch (const std::exception &e) {
//            std::cerr << "Exception : " << e.what() << "\n";
//        }
//        vkInfo("process import model successfully");
//
//        auto id = scene->mRootNode->mName.C_Str();
//        model.id = id;
//
//        auto rtInput = yic::Allocator::modelToGeometryKHR(model);
//        model.blass = yic::Allocator::buildBLAS(rtInput);
//
//        vk::TransformMatrixKHR identityMatrix;
//        glm::mat4 matrix{1.f};
//        auto temp = glm::transpose(matrix);
//        memcpy(&identityMatrix, &temp, sizeof(vk::TransformMatrixKHR));
//        std::vector<vk::AccelerationStructureInstanceKHR> asInsts;
//        for(auto& blas : model.blass){
//            vk::AccelerationStructureInstanceKHR asInst{
//                    identityMatrix, 0, 0xFF, 0,
//                    vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable, yic::Allocator::getAccelDevAddr(blas)
//            };
//            asInsts.emplace_back(asInst);
//        }
//        model.tlas = yic::Allocator::buildTLAS(asInsts);
//
//        return model;
//    }
//
//    auto ModelLoader::processMesh(const aiMesh *aiMesh, Mesh& mesh, const aiVector3D& center) -> void {
//        mesh.vertices.reserve(aiMesh->mNumVertices);
//
//        for (uint32_t j = 0; j < aiMesh->mNumVertices; j++) {
//            Vertex vertex{};
//
//            if (aiMesh->HasPositions()) {
//                auto& pos = aiMesh->mVertices[j];
//                vertex.pos = {pos.x - center.x, pos.y - center.y, pos.z - center.z};
//            }
//            if (aiMesh->HasNormals()) {
//                auto& nor = aiMesh->mNormals[j];
//                std::memcpy(&vertex.nor, &nor, sizeof (glm::vec3));
//            }
//            if (aiMesh->HasTextureCoords(0)) {
//                auto& uv = aiMesh->mTextureCoords[0][j];
//                std::memcpy(&vertex.uv, &uv, sizeof(glm::vec2));
//            }
//
//
//            mesh.vertices.emplace_back(vertex);
//        }
//
//        mesh.indices.reserve(aiMesh->mNumFaces);
//        for (uint32_t j = 0; j < aiMesh->mNumFaces; j++) {
//            auto face = aiMesh->mFaces[j];
//            for (uint32_t k = 0; k < face.mNumIndices; k++) {
//                mesh.indices.emplace_back(face.mIndices[k]);
//            }
//        }
//
//        mesh.vertBuf = yic::Allocator::allocBufStaging(sizeof(Vertex) * mesh.vertices.size(), mesh.vertices.data(), vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eStorageBuffer);
//        mesh.indexBuf = yic::Allocator::allocBufStaging(sizeof(uint32_t) * mesh.indices.size(), mesh.indices.data(), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eStorageBuffer);
//    }
//
//    auto ModelLoader::processMaterial(aiMaterial *aiMaterial, Model::Generic& model, Mesh& mesh, oneapi::tbb::concurrent_unordered_map<pt, uint32_t>& paths, const std::filesystem::path& modelDirPath) -> void {
//        bool texAlloc = false;
//        auto findOrAdd = [&](const pt& p) -> uint32_t {
//            std::string pathStr = p.string();
//            auto [iter, inserted] = paths.emplace(pathStr, 0);
//            if (!inserted) {
//                return iter->second;
//            } else {
//                uint32_t newIndex = paths.size() - 1;
//                iter->second = newIndex;
//                texAlloc = true;
//                return newIndex;
//            }
//        };
//        aiString path;
//        if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
//            pt texPath(path.C_Str());
//
//            auto loadPt = fo::findFileInDirectory(modelDirPath, texPath.filename());
//            auto index = findOrAdd(loadPt);
//            mesh.texIndex = index;
//            if (model.diffTexs.size() <= index){
//                //model.diffTexs.grow_by(index + 1 - model.diffTexs.size());
//                model.diffTexs.resize(index + 1);
//            }
//            if (texAlloc){
//                if (model.diffTexs[index] == nullptr) {
//                    model.diffTexs[index] = yic::Allocator::allocImg(loadPt.string());
//                }
//            }
//        }
//    }
//
//    auto ModelLoader::processMeshCenter(const aiScene* scene, Model::Generic& model) -> aiVector3D {
//        auto min = _mm256_set1_ps(FLT_MAX);
//        auto max = _mm256_set1_ps(FLT_MIN);
//
//        for(uint32_t i = 0; i < scene->mNumMeshes; i++){
//            auto mesh = scene->mMeshes[i];
//            auto ax = mesh->mAABB.mMax, in = mesh->mAABB.mMin;
//
//            auto meshMax = _mm256_set_ps(0, 0, 0, 0, ax.z, ax.y, ax.x, 0);
//            auto meshMin = _mm256_set_ps(0, 0, 0, 0, in.z, in.y, in.x, 0);
//
//            min = _mm256_min_ps(min, meshMin);
//            max = _mm256_max_ps(max, meshMax);
//
//            model.meshes[i].aabb = {{ax.x, ax.y, ax.z}, {in.x, in.y, in.z}};
//        }
//
//        float finalMin[8], finalMax[8];
//        _mm256_storeu_ps(finalMin, min);
//        _mm256_storeu_ps(finalMax, max);
//
//        model.aabb = {
//                {finalMax[4], finalMax[3], finalMax[2]},
//                {finalMin[4], finalMin[3], finalMin[2]}
//        };
//
//        return {
//          (finalMin[1] + finalMax[1]) / 2.f,
//          (finalMin[2] + finalMax[2]) / 2.f,
//          (finalMin[3] + finalMax[3]) / 2.f,
//        };
//    }
//
//    auto ModelLoader::buildDescriptor(Model::Generic &model, PipelineDesSetLayout &setLayout) -> void {
//        model.descriptor = std::make_shared<yic::Descriptor>(model.id, setLayout);
//        std::vector<std::shared_ptr<yic::vkImage>> texs(model.diffTexs.begin(), model.diffTexs.end());
//        model.descriptor->updateDesSet(model.diffTexs.size(), {yic::ImgInfo{texs}});
//    }
//
////    auto ModelLoader::Pmx(const std::string &path, PipelineDesSetLayout &setLayout) -> Model::Pmx{
////        std::string ResDir = saba::PathUtil::GetExecutablePath();
////        ResDir = saba::PathUtil::GetDirectoryName(ResDir);
////        ResDir = saba::PathUtil::Combine(ResDir, "resource");
////        std::string MmmDir = saba::PathUtil::Combine(ResDir, "mmd");
////
////        Model::Pmx pmx;
////
////        auto modelDirPath = std::filesystem::path(path).parent_path();
////
////        auto pmx_uptr = std::make_unique<saba::PMXModel>();
////
////        if (pmx_uptr->Load(path, MmmDir)){
////            pmx_uptr->InitializeAnimation();
////        }
////
////        const uint16_t* indices16 = nullptr;
////        uint32_t* indices32 = nullptr;
////        switch (pmx_uptr->GetIndexElementSize()) {
////            case 2:
////                indices16 = (const uint16_t*)pmx_uptr->GetIndices();
////
////                indices32 = new uint32_t[pmx_uptr->GetIndexCount()];
////                for(size_t i = 0; i < pmx_uptr->GetIndexCount(); i++){
////                    indices32[i] = static_cast<uint32_t>(indices16[i]);
////                }
////                break;
////            case 4:
////                indices32 = (uint32_t*)pmx_uptr->GetIndices();
////                break;
////        }
////
////        const auto& vertCount = pmx_uptr->GetVertexCount();
////        pmx_uptr->Update();
////        const auto &pos = pmx_uptr->GetUpdatePositions();
////        const auto &normals = pmx_uptr->GetUpdateNormals();
////        const auto &uvs = pmx_uptr->GetUpdateUVs();
////        const auto& subMeshes = pmx_uptr->GetSubMeshes();
////
////        oneapi::tbb::concurrent_unordered_map<std::string, uint32_t> paths;
////        bool texAlloc = false;
////        auto findOrAdd = [&](const pt& p) -> uint32_t {
////            std::string pathStr = p.string();
////            auto [iter, inserted] = paths.emplace(pathStr, 0);
////            if (!inserted) {
////                return iter->second;
////            } else {
////                uint32_t newIndex = paths.size() - 1;
////                iter->second = newIndex;
////                texAlloc = true;
////                return newIndex;
////            }
////        };
////
////        std::vector<Vertex> vertices;
////        for(size_t i = 0; i < pmx_uptr->GetVertexCount(); i++){
////            auto originUv = uvs[i];
////            auto flipUv = glm::vec2(originUv.x, 1.f - originUv.y);
////            vertices.emplace_back(Vertex{pos[i], normals[i], flipUv});
////        }
////
////        pmx.vertBuf = yic::Allocator::allocBufStaging(sizeof(Vertex) * vertices.size(), vertices.data(), vk::BufferUsageFlagBits::eVertexBuffer);
////        pmx.indexBuf = yic::Allocator::allocBufStaging(sizeof(uint32_t) * pmx_uptr->GetIndexCount(), indices32, vk::BufferUsageFlagBits::eIndexBuffer);
////        for(size_t i = 0; i < pmx_uptr->GetMaterialCount(); i++){
////            auto& pt = pmx_uptr->GetMaterials()[i].m_texture;
////            auto index = findOrAdd(pt);
////            pmx.texIndex.emplace_back(index);
////            if (pmx.diffTexs.size() <= index){
////                pmx.diffTexs.resize(index + 1);
////            }
////            if (texAlloc){
////                if (pmx.diffTexs[index] == nullptr){
////                    pmx.diffTexs[index] = yic::Allocator::allocImg(pt);
////                }
////            }
////        }
////
////        auto id = saba::PathUtil::GetFilename(path);
////        pmx.id = id;
////        pmx.descriptor = std::make_shared<yic::Descriptor>(id, setLayout);
////        pmx.descriptor->updateDesSet(pmx.diffTexs.size(), {yic::ImgInfo{pmx.diffTexs}});
////
////        pmx.pmx = std::move(pmx_uptr);
////
////        return pmx;
////    }
////
////    auto ModelLoader::Vmd(const std::string &path, Model::Pmx& pmx) -> void {
////        auto ani = std::make_unique<saba::VMDAnimation>();
////        if (!ani->Create(pmx.pmx)){
////            vkError("failed to create ani file");
////        }
////        saba::VMDFile vmd;
////        if (!saba::ReadVMDFile(&vmd, path.c_str())){
////            vkError("failed to read vmd file");
////        }
////        if (!ani->Add(vmd)){
////
////        }
////        ani->SyncPhysics(0.f, 30);
////        pmx.vmd = std::move(ani);
////    }
//
//
////    auto ModelLoader::processMeshMaterial(const aiMesh *aiMesh, const aiMaterial *aiMaterial,
////                                          const aiVector3D& center, Model::Generic &model,
////                                          oneapi::tbb::concurrent_unordered_map<pt, uint32_t> &paths,
////                                          const std::filesystem::path &modelDirPath) -> void {
////        bool texAlloc = false;
////        auto findOrAdd = [&](const pt& p) -> uint32_t {
////            std::string pathStr = p.string();
////            auto [iter, inserted] = paths.emplace(pathStr, 0);
////            if (!inserted) {
////                return iter->second;
////            } else {
////                uint32_t newIndex = paths.size() - 1;
////                iter->second = newIndex;
////                texAlloc = true;
////                return newIndex;
////            }
////        };
////        aiString path;
////        if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS){
////            pt texPath(path.C_Str());
////
////            auto loadPt = fo::findFileInDirectory(modelDirPath, texPath.filename());
////            auto index = findOrAdd(loadPt);
////            if (model.diffTexs.size() <= index){
////                model.diffTexs.resize(index + 1);
////                model.mesh.resize(index + 1);
////            }
////            if (texAlloc){
////                if (model.diffTexs[index] == nullptr) {
////                    model.diffTexs[index] = yic::Allocator::allocImg(loadPt.string());
////                }
////            }
////            for (uint32_t j = 0; j < aiMesh->mNumVertices; j++) {
////                Vertex vertex{};
////
////                if (aiMesh->HasPositions()) {
////                    auto& pos = aiMesh->mVertices[j];
////                    vertex.pos = {pos.x - center.x, pos.y - center.y, pos.z - center.z};
////                }
////                if (aiMesh->HasNormals()) {
////                    auto& nor = aiMesh->mNormals[j];
////                    std::memcpy(&vertex.nor, &nor, sizeof (glm::vec3));
////                }
////                if (aiMesh->HasTextureCoords(0)) {
////                    auto& uv = aiMesh->mTextureCoords[0][j];
////                    std::memcpy(&vertex.uv, &uv, sizeof(glm::vec2));
////                }
////
////                model.mesh[index].vertices.emplace_back(vertex);
////                model.mesh[index].texIndex = index;
////            }
////            auto increase = model.mesh[index].indices.size();
////            for (uint32_t j = 0; j < aiMesh->mNumFaces; j++) {
////                auto face = aiMesh->mFaces[j];
////                for (uint32_t k = 0; k < face.mNumIndices; k++) {
////                    model.mesh[index].indices.emplace_back(face.mIndices[k] + increase);
////                }
////            }
////        }
////    }

} // sc