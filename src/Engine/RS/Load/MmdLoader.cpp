//
// Created by lenovo on 6/13/2025.
//

#include "MmdLoader.h"
#include <saba/src/Saba/Base/Path.h>
#include <saba/src/Saba/Model/MMD/PMDModel.h>
#include <saba/src/Saba/Model/MMD/VMDFile.h>

#include "Core/Management/TripleBufferIndexManager.h"
#include "RHI/Allocator.h"
#include "RHI/DescriptorSystem.h"
#include "Runetime/System/RenderLibrary.h"

namespace rs {

    auto MmdLoader::Load(const vot::string &pt, vot::BasicInfoComponent &bic,
                            vot::VertexDataComponent &vdc, vot::RenderComponent &rc) -> vot::string {
        auto pmx = initPmx(pt, bic, vdc);

        buildVertices(pmx.get(), vdc, rc);
        buildIndices(pmx.get(), rc);
        buildMaterials(pmx.get(), rc);
        buildSubMeshes(pmx.get(), rc);
        buildDS(rc);

        vdc.pmx = std::move(pmx);
        return bic.name;
    }


    auto MmdLoader::LoadVmd(const vot::string &pt) -> void {
        saba::VMDFile vmdFile;
        if (!saba::ReadVMDFile(&vmdFile, pt.c_str())) {
            yic::logger->error("failed to read vmd file: {}", pt);
        }
        const std::filesystem::path path(pt);
        vmdFiles.emplace_back(path.stem().string(), std::move(vmdFile));
    }

    auto MmdLoader::initPmx(const vot::string &pt, vot::BasicInfoComponent &bic, vot::VertexDataComponent &vdc) -> std::unique_ptr<saba::PMXModel> {
        mResDir = saba::PathUtil::GetExecutablePath();
        mResDir = saba::PathUtil::GetDirectoryName(mResDir);
        mResDir = saba::PathUtil::Combine(mResDir, "resource");
        mMmdDir = saba::PathUtil::Combine(mResDir, "mmd");

        const std::filesystem::path path(pt);
        bic.name = path.stem().string();

        auto pmx = std::make_unique<saba::PMXModel>();
        if(!pmx->Load(pt.data(), mMmdDir)){
            yic::logger->error("failed to load pmx model");
        } else { yic::logger->info("load pmx model"); }

        //vdc.isMMD = true;
        pmx->InitializeAnimation();

        return std::move(pmx);
    }

    auto MmdLoader::buildVertices(saba::PMXModel* pmx, vot::VertexDataComponent& vdc, vot::RenderComponent &rc) -> void {
        pmx->Update();
        const auto pos = pmx->GetUpdatePositions();
        const auto nor = pmx->GetUpdateNormals();
        const auto uv = pmx->GetUpdateUVs();

        for(auto& pmr : vdc.mmd_vertices_pmr){
            pmr = std::pmr::vector<vot::VertexT<vot::eMMD>>{&mVertexPool};
            pmr.resize(pmx->GetVertexCount());
        }

        vot::vector<vot::VertexT<vot::eMMD>> vertices;
        vot::AABB aabb{};
        vertices.resize(pmx->GetVertexCount());
        for(auto i = 0; i < pmx->GetVertexCount(); i++){
            vertices[i] = vot::VertexT<vot::eMMD>{pos[i], nor[i], uv[i]};
            aabb.min = glm::min(aabb.min, pos[i]);
            aabb.max = glm::max(aabb.max, pos[i]);
        }
        rc.center = ((aabb.min + aabb.max) / 2.f);
        rc.baseMat = glm::translate(glm::mat4 (1.f), -rc.center);

        for(auto& v : rc.vertexBuffer){
            v = yic::allocator->allocBufferStaging(pmx->GetVertexCount() * sizeof(vot::VertexT<vot::eMMD>), vertices.data(),
                vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eVertexBuffer, "pmx vert");
        }
    }

    auto MmdLoader::buildIndices(const saba::PMXModel *pmx, vot::RenderComponent &rc) -> void {
        vot::vector<uint32_t > indices;
        indices.reserve(pmx->GetIndexCount());
        switch (pmx->GetIndexElementSize()) {
            case 2:
                rc.indexType = vk::IndexType::eUint16;
                break;
            case 4:
                rc.indexType = vk::IndexType::eUint32;
                break;
            default:
                yic::logger->warn("unknown index size {0}", pmx->GetIndexElementSize());
                break;
        }
        rc.indexBuffer = yic::allocator->allocBufferStaging(pmx->GetIndexCount() * pmx->GetIndexElementSize(), pmx->GetIndices(),
            vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eIndexBuffer, "index buf");
    }

    auto MmdLoader::buildMaterials(const saba::PMXModel *pmx, vot::RenderComponent &rc) -> void {
        rc.diffuseTextures.resize(pmx->GetMaterialCount());
        std::unordered_map<std::string, vot::Image_sptr> mats;
        for(auto i = 0; i < pmx->GetMaterialCount(); i++){
            auto& ptMat = pmx->GetMaterials()[i].m_texture;
            if (mats.find(ptMat) == mats.end()){
                mats[ptMat] = yic::allocator->loadTexture(ptMat.c_str());
            }

            if (mats[ptMat]){
                rc.diffuseTextures[i] = mats[ptMat];
            }
        }
    }

    auto MmdLoader::buildSubMeshes(const saba::PMXModel* pmx, vot::RenderComponent &rc) -> void {
        vot::vector<saba::MMDSubMesh> subMesh(pmx->GetSubMeshCount());
        for(auto i = 0; i < pmx->GetSubMeshCount(); i++){
            subMesh[i] = pmx->GetSubMeshes()[i];
        }
        for(auto i = 0; i < subMesh.size(); i++){
            rc.subMeshes[i].emplace_back(subMesh[i].m_vertexCount, subMesh[i].m_beginIndex);
        }
    }

    auto MmdLoader::buildDS(vot::RenderComponent &rc) -> void {
        rc.dsHandle = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};

            for(const auto& img : rc.diffuseTextures){
                layout.emplace(vot::DescriptorLayout2::_1d{
                        img->imageInfo(),
                });
            }

            return layout;
        }, yic::renderLibrary->GP_Basic_PMX);
    }
} // sc