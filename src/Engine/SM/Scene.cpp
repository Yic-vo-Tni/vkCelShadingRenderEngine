//
// Created by lenovo on 11/21/2024.
//

#include "Runtime/System/RenderLibrary.h"
#include "RHI/DescriptorSystem.h"
#include "Scene.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
#include "RHI/GpuRuntime/Alloctor/Allocator.h"
#include "RHI/Command.h"
#include "RS/ResourceSystem.h"
#include "Runtime/Camera/Camera.h"

namespace sm {

    SceneSystem::SceneSystem(entt::registry& registry) : ecs(registry) {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();

        loadScene();
        buildSet0();
    }

    auto SceneSystem::loadScene(const vot::string &id) -> void {
        const auto effId = id.empty() ? "scene " + vot::string(std::to_string(mScenes.size())) : id;
        mActiveScene = &mScenes.try_emplace(effId, Scene{id}).first->second;
    }

    auto SceneSystem::unloadScene(const vot::string &id) -> void {
        mScenes.erase(id);
    }

    auto SceneSystem::reloadTlas() -> void {
        mActiveScene->update = false;

        vot::vector<std::array<uint64_t, 2>> bufferAddr;

        ecs.view<const vot::RenderComponent>()
                .each([&](auto e, const vot::RenderComponent& rc){
                   bufferAddr.emplace_back(std::array<uint64_t, 2>{rc.vertexBuffer[yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur()]->bufferAddr(), rc.indexBuffer->bufferAddr()});
                });
        if (bufferAddr.empty()) return;
        mActiveScene->bufferAddrBuffer = yic::allocator->allocBufferStaging(sizeof (uint64_t ) * 2 * bufferAddr.size(), bufferAddr.data(), vk::BufferUsageFlagBits::eStorageBuffer);

        syncTLAS();
        // FIXME:
        // When models are loaded asynchronously, this descriptor set may be updated once
        // (TLAS / storage buffer bindings) while the main render command buffer from a
        // previous frame is still in-flight.
        //
        // This causes update-after-bind validation errors under Vulkan,
        // especially for ray tracing pipelines.
    }

    auto SceneSystem::frame() -> void {
        static double elapsedTime = 0.0;
        //elapsedTime += GLOBAL::dt;
        elapsedTime += GLOBAL::dt.load(std::memory_order_relaxed);

        auto playAnim = false;
        ecs.view<const vot::BasicInfoComponent, vot::VertexDataComponent, vot::RenderComponent, vot::RayTracingComponent, vot::AnimationComponent>()
        .each([&](const entt::entity, const vot::BasicInfoComponent &bc, const vot::VertexDataComponent &vc,
                const vot::RenderComponent &rc, vot::RayTracingComponent &rtc, const vot::AnimationComponent &ac) {
            if (ac.enableAnim && (GLOBAL::playAllAnim || bc.playAnimation)){
                bool onlyTransform = true;

                if (elapsedTime >= 2.0) { onlyTransform = false; elapsedTime = 0.0;}

                syncBLAS(vc, rc, rtc, onlyTransform);

                playAnim = true;
            }
        });

        if (GLOBAL::visibleZMO || playAnim) {
            syncTLAS();
        }
    }

    auto SceneSystem::buildSet0() -> void {
        auto& cam = GLOBAL::entity::camera.make_va<sc::Camera>();
        cam.computeViewProjMatrix();

        vot::comp::Light::Meta meta{};
        meta.entries.resize(30); // HACK: dynamic increase
        meta.ssbo = yic::allocator->allocBuffer(sizeof(vot::comp::Light::Entry) * 30, vk::BufferUsageFlagBits::eStorageBuffer, "Light entries");
        meta.ssbo->update(meta.entries);

        vot::BasicInfoComponent basicInfoComponent{};
        vot::VertexDataComponent vertexDataComponent{};
        vot::RenderComponent renderComponent{};
        vot::AnimationComponent animationComponent{};

        yic::resourceSystem->mLoader->mAssimpLoader->Load(tex_path "../Model/Light/Sphere.gltf", basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
        // FIXME: tex_path -> mod_path

        vot::EntityView<vot::bit::eEntityViewDefault>::create()
            .mark<vot::mark::eVisible>()
            .emplace(meta, basicInfoComponent, vertexDataComponent, renderComponent);

        auto& [set0] = GLOBAL::entity::set0.make_va<vot::DescriptorSet0>();
        std::ranges::for_each(std::views::iota(0, 3), [&](auto i) -> void {
            set0[i] = yic::desSystem->allocUpdateDescriptorSets([&](vot::DescriptorLayout2& layout2) {
                layout2.emplace({cam.vpBufferInfo(i), meta.ssbo->bufferInfo()});
            }, yic::renderLibrary->GP_Basic_Assimp, 0, 1);
        });
    }

    auto SceneSystem::syncBLAS(const vot::VertexDataComponent &vc, const vot::RenderComponent &rc,
                                vot::RayTracingComponent &rtc, const bool update) -> void {
        auto vertAddr = rc.vertexBuffer[yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur()]->bufferAddr();
        const auto indexAddr = rc.indexBuffer->bufferAddr();

        uint32_t maxVert{}, numTri{}, stride{};
        vk::IndexType indexType{};

        if (vc.type == vot::eMMD){
            maxVert = static_cast<uint32_t>(vc.pmx->GetVertexCount());
            numTri = static_cast<uint32_t>(vc.pmx->GetIndexCount()) / 3;
            stride = sizeof(vot::VertexT<vot::eMMD>);
            indexType = rc.indexType;
        } else {
            vertAddr = rc.vertexBuffer[vot::VertexDataComponent::eAnim]->bufferAddr();
            maxVert = static_cast<uint32_t>(vc.vertices_pmr[vot::VertexDataComponent::eTPose].size());
            numTri = static_cast<uint32_t>(vc.indices_pmr.size()) / 3;
            stride = sizeof (vot::VertexT<vot::eAssimp>);
            indexType = vk::IndexType::eUint32;
        }

        const auto asGeomTriData = vk::AccelerationStructureGeometryTrianglesDataKHR()
                .setVertexFormat(vk::Format::eR32G32B32Sfloat)
                .setVertexData(vertAddr)
                .setMaxVertex(maxVert)
                .setIndexData(indexAddr)
                .setVertexStride(stride)
                .setIndexType(indexType);

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
                                vk::PipelineStageFlagBits::eRayTracingShaderKHR,
                                {}, barrier, {}, {});
        });
    }

    auto SceneSystem::syncTLAS() -> void {
        vot::vector<vk::AccelerationStructureInstanceKHR> asInsts;

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
                yic::renderLibrary->RTX_RayTracing->imageInfo(std::nullopt, nullptr, vk::ImageLayout::eGeneral),
                mActiveScene->bufferAddrBuffer->bufferInfo(),
        }};};
        if (DS_RP_Shadow.pSet == nullptr){
            DS_RP_Shadow = yic::desSystem->allocUpdateDescriptorSets(layout,yic::renderLibrary->RP_Shadow);
        } else {
            yic::desSystem->updateDescriptorSets(layout, yic::renderLibrary->RP_Shadow, DS_RP_Shadow);
        }
        // BUG:
        // Ray tracing descriptor set is currently a single shared instance across frames.
        // TLAS updates here implicitly require descriptor updates, which are unsafe
        // while previous frames are still in-flight.
        //
        // Proper fix:
        // - Allocate RT descriptor sets per-frame (per in-flight frame)
        // - Update them in a round-robin fashion (e.g. triple buffering)
        //
        // This will be fixed when migrating to a per-frame descriptor model
        // or descriptor buffers.
    }

} // sm