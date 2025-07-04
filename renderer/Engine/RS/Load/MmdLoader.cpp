//
// Created by lenovo on 6/13/2025.
//

#include "MmdLoader.h"
#include <saba/src/Saba/Base/Path.h>
#include <saba/src/Saba/Base/File.h>
#include <saba/src/Saba/Base/UnicodeUtil.h>
#include <saba/src/Saba/Base/Time.h>
#include <saba/src/Saba/Model/MMD/PMDModel.h>
#include <saba/src/Saba/Model/MMD/VMDFile.h>
#include <saba/src/Saba/Model/MMD/VMDAnimation.h>
#include <saba/src/Saba/Model/MMD/VMDCameraAnimation.h>

#include "RHI/Allocator.h"
#include "RHI/DescriptorSystem.h"
#include "ECS/System/RenderLibrary.h"

namespace rs {

    auto MmdLoader::Load(const vot::string &pt, vot::BasicInfoComponent &basicInfoComponent,
                            vot::VertexDataComponent &vertexDataComponent,
                            vot::RenderComponent &renderComponent) -> vot::string {
        mResDir = saba::PathUtil::GetExecutablePath();
        mResDir = saba::PathUtil::GetDirectoryName(mResDir);
        mResDir = saba::PathUtil::Combine(mResDir, "resource");
        mMmdDir = saba::PathUtil::Combine(mResDir, "mmd");

        std::filesystem::path path(pt);
        basicInfoComponent.name = path.stem().string();

        auto pmx = std::make_unique<saba::PMXModel>();
        if(!pmx->Load(pt.data(), mMmdDir)){
            yic::logger->error("failed to load pmx model");
        } else { yic::logger->info("load pmx model"); }

        vertexDataComponent.isMMD = true;
        pmx->InitializeAnimation();

        pmx->Update();
        auto pos = pmx->GetUpdatePositions();
        auto nor = pmx->GetUpdateNormals();
        auto uv = pmx->GetUpdateUVs();

        vot::vector<Vertex> vertices;
        vot::AABB aabb{};
        vertices.resize(pmx->GetVertexCount());
        for(auto i = 0; i < pmx->GetVertexCount(); i++){
            vertices[i] = Vertex{pos[i], nor[i], uv[i]};
            aabb.min = glm::min(aabb.min, pos[i]);
            aabb.max = glm::max(aabb.max, pos[i]);
        }
        renderComponent.center = ((aabb.min + aabb.max) / 2.f);
        renderComponent.baseMat = glm::translate(glm::mat4 (1.f), -renderComponent.center);

        auto usage = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress;
        renderComponent.vertexBuffer = yic::allocator->allocBufferStaging(pmx->GetVertexCount() * sizeof(Vertex), vertices.data(), usage | vk::BufferUsageFlagBits::eVertexBuffer, "pmx vert");
        vot::vector<uint32_t > indices;
        indices.reserve(pmx->GetIndexCount());
        switch (pmx->GetIndexElementSize()) {
            case 2:
                renderComponent.indexType = vk::IndexType::eUint16;
                break;
            case 4:
                renderComponent.indexType = vk::IndexType::eUint32;
                break;
            default:
                yic::logger->warn("unknown index size {0}", pmx->GetIndexElementSize());
                break;
        }
        renderComponent.indexBuffer = yic::allocator->allocBufferStaging(pmx->GetIndexCount() * pmx->GetIndexElementSize(), pmx->GetIndices(), usage | vk::BufferUsageFlagBits::eIndexBuffer, "index buf");

        vot::vector<saba::MMDSubMesh> subMesh(pmx->GetSubMeshCount());
        for(auto i = 0; i < pmx->GetSubMeshCount(); i++){
            subMesh[i] = pmx->GetSubMeshes()[i];
        }

        renderComponent.diffuseTextures.resize(pmx->GetMaterialCount());
        std::unordered_map<std::string, vot::Image_sptr> mats;
        for(auto i = 0; i < pmx->GetMaterialCount(); i++){
            auto& ptMat = pmx->GetMaterials()[i].m_texture;
            if (mats.find(ptMat) == mats.end()){
                mats[ptMat] = yic::allocator->loadTexture(ptMat.c_str());
            }

            if (mats[ptMat]){
                renderComponent.diffuseTextures[i] = mats[ptMat];
            }
        }


        for(auto i = 0; i < subMesh.size(); i++){
            renderComponent.subMeshes[i].emplace_back(subMesh[i].m_vertexCount, subMesh[i].m_beginIndex);
        }


        renderComponent.dsHandle = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};

            for(const auto& img : renderComponent.diffuseTextures){
                layout.emplace(vot::DescriptorLayout2::_1d{
                        img->imageInfo(),
                });
            }

            return layout;
        }, yic::renderLibrary->GP_Basic_PMX);

        vertexDataComponent.pmx = std::move(pmx);

        return basicInfoComponent.name;
    }

    auto MmdLoader::bindVmd(const vot::string &pt, const vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void {
        auto vmd = std::make_unique<saba::VMDAnimation>();
        if (!vmd->Create(vc.pmx)){
            yic::logger->error("failed to load vmd");
        }

        saba::VMDFile vmdFile;
        if (!saba::ReadVMDFile(&vmdFile, pt.c_str())){

        }
        if (!vmd->Add(vmdFile)){

        }
        vmd->SyncPhysics(0.f);

        std::filesystem::path path(pt);
        ac.vmd = std::pair(path.stem().string(), std::move(vmd));
//        ac.vmds.emplace_back(path.stem().string(), std::move(vmd));
    }




    auto MmdLoader::bakeVmd(vot::VertexDataComponent& vc, vot::AnimationComponent& ac) -> void {
        auto AnimTime = 0.f;
        auto elapsed = 1.f / 30.f;

        bakeVertices.resize(ac.vmd.second->GetMaxKeyTime() + 1);
        for (auto i = 0u; i < ac.vmd.second->GetMaxKeyTime(); i++) {
            bakeVertices[i].resize(vc.pmx->GetVertexCount());
            vc.pmx->BeginAnimation();
            vc.pmx->UpdateAllAnimation(ac.vmd.second.get(), (float)i, 1 / 30.f);
            vc.pmx->EndAnimation();

            vc.pmx->Update();
            auto pos = vc.pmx->GetUpdatePositions();
            auto nor = vc.pmx->GetUpdateNormals();
            auto uv = vc.pmx->GetUpdateUVs();
            for(auto j = 0; j < vc.pmx->GetVertexCount(); j++){
                bakeVertices[i][j] = vot::Vertex{.pos = pos[j], .nor = nor[j], .uv = uv[j]};
            }
        }

        yic::logger->info("bake vmd success");
    }

    auto MmdLoader::updateAnim(const vot::VertexDataComponent& vc, vot::RenderComponent& rc) -> void {
        vc.pmx->Update();
        auto pos = vc.pmx->GetUpdatePositions();
        auto nor = vc.pmx->GetUpdateNormals();
        auto uv = vc.pmx->GetUpdateUVs();

        vot::vector<Vertex> vertices;
        vertices.resize(vc.pmx->GetVertexCount());
        for(auto i = 0; i < vc.pmx->GetVertexCount(); i++){
            vertices[i] = Vertex{pos[i], nor[i], uv[i]};
        }
//        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, vertices.size(), 1024), [&](const oneapi::tbb::blocked_range<size_t>& r){
//            for(size_t i = r.begin(); i < r.end(); i++){
//                vertices[i].pos = pos[i];
//                vertices[i].nor = nor[i];
//                vertices[i].uv = uv[i];
//            }
//        });

        rc.vertexBuffer->update(vertices);
    }

    auto MmdLoader::vmd(const vot::string &pt) -> void {
        ptVmds.emplace_back(pt);
    }


} // sc