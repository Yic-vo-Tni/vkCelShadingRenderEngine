//
// Created by lenovo on 8/7/2024.
//

#include "PmxLoader.h"
#include "Engine/RHI/Allocator.h"

namespace sc {


//    PmxLoader::PmxLoader() {
//        mResDir = saba::PathUtil::GetExecutablePath();
//        mResDir = saba::PathUtil::GetDirectoryName(mResDir);
//        mResDir = saba::PathUtil::Combine(mResDir, "resource");
//        mMmmDir = saba::PathUtil::Combine(mResDir, "mmd");
//    }
//
//    auto PmxLoader::Load(const std::string &path, PipelineDesSetLayout &setLayout) -> Pmx {
//        Pmx pmx;
//
//        auto modelDirPath = std::filesystem::path(path).parent_path();
//
//        auto pmx_uptr = std::make_unique<saba::PMXModel>();
//
//        if (pmx_uptr->Load(path, get()->mMmmDir)){
//            pmx_uptr->InitializeAnimation();
//        }
//
//        const uint16_t* indices16 = nullptr;
//        uint32_t* indices32 = nullptr;
//        switch (pmx_uptr->GetIndexElementSize()) {
//            case 2:
//                indices16 = (const uint16_t*)pmx_uptr->GetIndices();
//
//                indices32 = new uint32_t[pmx_uptr->GetIndexCount()];
//                for(size_t i = 0; i < pmx_uptr->GetIndexCount(); i++){
//                    indices32[i] = static_cast<uint32_t>(indices16[i]);
//                }
//                break;
//            case 4:
//                indices32 = (uint32_t*)pmx_uptr->GetIndices();
//                break;
//        }
//
//        const auto& vertCount = pmx_uptr->GetVertexCount();
//        pmx_uptr->Update();
//        const auto &pos = pmx_uptr->GetUpdatePositions();
//        const auto &normals = pmx_uptr->GetUpdateNormals();
//        const auto &uvs = pmx_uptr->GetUpdateUVs();
//        const auto& subMeshes = pmx_uptr->GetSubMeshes();
//
//        oneapi::tbb::concurrent_unordered_map<std::string, uint32_t> paths;
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
//
//        std::vector<Vertex> vertices;
//        for(size_t i = 0; i < pmx_uptr->GetVertexCount(); i++){
//            auto originUv = uvs[i];
//            auto flipUv = glm::vec2(originUv.x, 1.f - originUv.y);
//            vertices.emplace_back(Vertex{pos[i], normals[i], flipUv});
//        }
//
//         pmx.vertBuf = yic::Allocator::allocBufStaging(sizeof(Vertex) * vertices.size(), vertices.data(), vk::BufferUsageFlagBits::eVertexBuffer, yic::Allocator::MemoryUsage::eGpuOnly, IdGenerator::uniqueId());
//         pmx.indexBuf = yic::Allocator::allocBufStaging(sizeof(uint32_t) * pmx_uptr->GetIndexCount(), indices32, vk::BufferUsageFlagBits::eIndexBuffer, yic::Allocator::MemoryUsage::eGpuOnly, IdGenerator::uniqueId());
//        for(size_t i = 0; i < pmx_uptr->GetMaterialCount(); i++){
//            auto& pt = pmx_uptr->GetMaterials()[i].m_texture;
//            auto index = findOrAdd(pt);
//            pmx.texIndex.emplace_back(index);
//            if (pmx.diffTexs.size() <= index){
//                pmx.diffTexs.resize(index + 1);
//            }
//            if (texAlloc){
//                if (pmx.diffTexs[index] == nullptr){
//                    pmx.diffTexs[index] = yic::Allocator::allocImg(pt);
//                }
//            }
//        }
//
//        auto id = saba::PathUtil::GetFilename(path);
//        pmx.id = id;
//        pmx.descriptor = std::make_shared<yic::Descriptor>(id, setLayout);
//        pmx.descriptor->updateDesSet(pmx.diffTexs.size(), {yic::ImgInfo{pmx.diffTexs}});
//
//        pmx.pmx = std::move(pmx_uptr);
//
//        return pmx;
//    }

//    VmdLoader::VmdLoader(Pmx &pmx, const std::string &path) {
//        auto vmd = std::make_unique<saba::VMDAnimation>();
//        if (!vmd->Create(pmx.pmx)){
//
//        }
//        saba::VMDFile vmdFile;
//        saba::ReadVMDFile(&vmdFile, path.c_str());
//        vmd->Add(vmdFile);
//        vmd->SyncPhysics(0.f);
//        pmx.vmd = std::move(vmd);
//    }
} // yic