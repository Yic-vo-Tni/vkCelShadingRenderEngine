//
// Created by lenovo on 11/21/2024.
//

#include "ECS/System/RenderLibrary.h"
#include "RHI/DescriptorSystem.h"
#include "Scene.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "RHI/Command.h"

namespace sm {

    SceneSystem::SceneSystem(entt::registry& registry) : ecs(registry) {
    //SceneSystem::SceneSystem(flecs::world &ecs) : ecs(ecs) {
        ct = yic::systemHub.val<ev::pVkSetupContext>();
        rt = yic::systemHub.val<ev::pVkRenderContext>();

        loadScene();
    }

    auto SceneSystem::loadScene(const vot::string &id) -> void {
        auto effId = id.empty() ? "scene " + vot::string(std::to_string(mScenes.size())) : id;
        mActiveScene = &mScenes.try_emplace(effId, Scene{id}).first->second;
    }

    auto SceneSystem::unloadScene(const vot::string &id) -> void {
        mScenes.erase(id);
    }

    auto SceneSystem::reloadTlas() -> void {
        mActiveScene->update = false;

        vot::vector<std::array<uint64_t, 2>> bufferAddr;
//        ecs.query<const vot::RenderComponent>().each([&](flecs::entity e, const vot::RenderComponent& rc){
//            bufferAddr.emplace_back(std::array<uint64_t, 2>{rc.vertexBuffer->bufferAddr(), rc.indexBuffer->bufferAddr()});
//        });
        ecs.view<const vot::RenderComponent>()
                .each([&](auto e, const vot::RenderComponent& rc){
                   bufferAddr.emplace_back(std::array<uint64_t, 2>{rc.vertexBuffer->bufferAddr(), rc.indexBuffer->bufferAddr()});
                });
        mActiveScene->bufferAddrBuffer = yic::allocator->allocBufferStaging(sizeof (uint64_t ) * 2 * bufferAddr.size(), bufferAddr.data(), vk::BufferUsageFlagBits::eStorageBuffer);

        syncTLAS();
    }

    auto SceneSystem::frame() -> void {
//        if (*rt.activeImageIndex == 0) {
            ecs.view<const vot::BasicInfoComponent, vot::VertexDataComponent, vot::RenderComponent, vot::RayTracingComponent>()
                    .each([&](const entt::entity &e, const vot::BasicInfoComponent &bc, vot::VertexDataComponent &vc,
                              vot::RenderComponent &rc, vot::RayTracingComponent &rtc) {
                        if (bc.playAnimation)
                            syncBLAS(vc, rc, rtc, true);
                    });
            if (GLOBAL::visibleZMO) {
                syncTLAS();
            }
//        }
    }

    auto SceneSystem::syncBLAS(const vot::VertexDataComponent &vc, vot::RenderComponent &rc,
                                vot::RayTracingComponent &rtc, bool update) -> void {
        auto vertAddr = rc.vertexBuffer->bufferAddr();
        auto indexAddr = rc.indexBuffer->bufferAddr();

        uint32_t maxVert{}, numTri{};
        if (vc.isMMD){
            maxVert = (uint32_t ) vc.pmx->GetVertexCount();
            numTri = (uint32_t ) vc.pmx->GetIndexCount() / 3;
        } else {
            maxVert = (uint32_t) vc.vertices_pmr.size();
            numTri = (uint32_t) vc.indices_pmr.size() / 3;
        }

        auto asGeomTriData = vk::AccelerationStructureGeometryTrianglesDataKHR()
                .setVertexFormat(vk::Format::eR32G32B32Sfloat)
                .setVertexData(vertAddr)
                .setMaxVertex(maxVert)
                .setIndexData(indexAddr);
        if (vc.isMMD){
            asGeomTriData.setVertexStride(offsetof(vot::Vertex, boneIds))
            .setIndexType(rc.indexType);
        } else {
            asGeomTriData.setVertexStride(sizeof (vot::Vertex))
            .setIndexType(vk::IndexType::eUint32);
        }

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setTriangles(asGeomTriData));

        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = ct.device->getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTri, *ct.dynamicDispatcher);

        if (!update){
            rtc.blas = yic::allocator->allocAccel(asBuildSizeInfo.accelerationStructureSize,
                                                  vk::AccelerationStructureTypeKHR::eBottomLevel);

            rtc.scratchBuffer = yic::allocator->allocDedicatedBufferStaging(asBuildSizeInfo.buildScratchSize,
                                                                          vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                                          vk::BufferUsageFlagBits::eStorageBuffer,
                                                                          " blas scratch");

            asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                    .setDstAccelerationStructure(rtc.blas->accel)
                    .setScratchData(rtc.scratchBuffer->bufferAddr());
        } else {
            asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eUpdate)
                .setSrcAccelerationStructure(rtc.blas->accel)
                .setDstAccelerationStructure(rtc.blas->accel)
                .setScratchData(rtc.scratchBuffer->bufferAddr());
        }

        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setFirstVertex(0)
                .setPrimitiveOffset(0)
                .setPrimitiveCount(numTri)
                .setTransformOffset(0);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
            cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, *ct.dynamicDispatcher);
            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                      vk::AccessFlagBits::eAccelerationStructureReadKHR};
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                {}, barrier, {}, {});
        });
    }

    auto SceneSystem::syncTLAS() -> void {
        vot::vector<vk::AccelerationStructureInstanceKHR> asInsts;
//        ecs.query<const vot::RayTracingComponent, const vot::RenderComponent>().each([&](flecs::entity e, const vot::RayTracingComponent& rtc, const vot::RenderComponent& rc){
//            asInsts.emplace_back(vk::AccelerationStructureInstanceKHR()
//                    .setTransform(yic::allocator->glmMatToVkTransformMatrix(rc.baseMat * rc.zmoMat))
//                    .setInstanceCustomIndex(0)
//                    .setMask(0xff)
//                    .setInstanceShaderBindingTableRecordOffset(0)
//                    .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
//                    .setAccelerationStructureReference(rtc.blas->accelAddr()));
//        });
        ecs.view<const vot::RayTracingComponent, const vot::RenderComponent>()
                .each([&](auto e, const vot::RayTracingComponent &rtc, const vot::RenderComponent &rc) {
                    asInsts.emplace_back(vk::AccelerationStructureInstanceKHR()
                                                 .setTransform(yic::allocator->glmMatToVkTransformMatrix(rc.baseMat * rc.zmoMat))
                                                 .setInstanceCustomIndex(0)
                                                 .setMask(0xff)
                                                 .setInstanceShaderBindingTableRecordOffset(0)
                                                 .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                                                 .setAccelerationStructureReference(rtc.blas->accelAddr()));
                });
        if (asInsts.empty()) return;

        if (!mActiveScene->update){
            mActiveScene->instBuffer = yic::allocator->allocBufferStaging(sizeof (vk::AccelerationStructureInstanceKHR) * asInsts.size(), asInsts.data(),
                                                          vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                          vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
        } else { mActiveScene->instBuffer->update(asInsts); }

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setGeometryType(vk::GeometryTypeKHR::eInstances)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setInstances(vk::AccelerationStructureGeometryInstancesDataKHR()
                .setArrayOfPointers(vk::False)
                .setData(mActiveScene->instBuffer->bufferAddr())));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace | vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = ct.device->getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, asInsts.size(), *ct.dynamicDispatcher);

        if (!mActiveScene->update) mActiveScene->tlas = yic::allocator->allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eTopLevel);

        if (!mActiveScene->update) mActiveScene->scratchBuffer = yic::allocator->allocDedicatedBufferStaging(asBuildSizeInfo.buildScratchSize,
                                                             vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);

        asBuildGeomInfo.setMode(mActiveScene->update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(mActiveScene->tlas->accel)
                .setScratchData(mActiveScene->scratchBuffer->bufferAddr());
        if (mActiveScene->update) asBuildGeomInfo.setSrcAccelerationStructure(mActiveScene->tlas->accel);

        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setPrimitiveCount(asInsts.size())
                .setTransformOffset(0)
                .setPrimitiveOffset(0)
                .setFirstVertex(0);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
            cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, *ct.dynamicDispatcher);
            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR, vk::AccessFlagBits::eAccelerationStructureReadKHR};
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                {}, barrier, {}, {});
        });

        if (!mActiveScene->update) mActiveScene->update = true;
        updateDS();
    }

    auto SceneSystem::updateDS() -> void {
        auto& DS_RP_Shadow = yic::renderLibrary->RP_Shadow.DS;
        auto layout = [&]{ return vot::DescriptorLayout2{ vot::DescriptorLayout2::_1d{
                mActiveScene->tlas->accelInfo(),
                yic::renderLibrary->RT_RayTracing->imageInfo(std::nullopt, nullptr, vk::ImageLayout::eGeneral),
                mActiveScene->bufferAddrBuffer->bufferInfo(),
        }};};
        if (DS_RP_Shadow.pSet == nullptr){
            DS_RP_Shadow = yic::desSystem->allocUpdateDescriptorSets(layout,yic::renderLibrary->RP_Shadow);
        } else {
            yic::desSystem->updateDescriptorSets(layout, yic::renderLibrary->RP_Shadow, DS_RP_Shadow);
        }
    }

} // sm