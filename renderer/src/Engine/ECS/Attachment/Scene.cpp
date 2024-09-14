//
// Created by lenovo on 8/30/2024.
//

#include "Scene.h"

#include <ranges>
#include "Engine/RHI/Allocator.h"
#include "Engine/RHI/Command.h"

namespace sc {

    SceneManager::SceneManager() {
        auto ct = mg::SystemHub.val<ev::pVkSetupContext>();
        mDevice = *ct.device;
        mDyDispatcher = *ct.dynamicDispatcher;
        mRenderHandle = mg::SystemHub.val<ev::pRenderProcess>(toolkit::enum_name(RenderPhase::ePrimary)).rp;

        mRTOffImage = mg::Allocator->allocImage(yic2::ImageConfig()
                .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc)
                .setExtent(mExtent)
                .setDstImageLayout(vk::ImageLayout::eGeneral), "off: rt");
        mRenderTargetOffImage = mg::Allocator->allocImage(yic2::ImageConfig()
                .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
                .setExtent(mExtent)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "off: rt shader read");

        loadScene();
        cRTRenderGroup();
        render();
    }

    auto SceneManager::loadScene(const std::string &id) -> void {
        auto effId = id.empty() ? "scene" + std::to_string(mSceneCount) : id;
        auto& scene = mScenes.try_emplace(effId, Scene{id}).first->second;
        mActiveScene = &scene;
        mSceneCount ++;
    }

    auto SceneManager::unloadScene(const std::string &id) -> void {
        mScenes.erase(id);
    }

    auto SceneManager::switchScene(const std::string &id) -> void {
        if (mScenes.find(id) != mScenes.end()){
            mActiveScene = &mScenes.at(id);
        } else {
            vkWarn("can't find the scene {0}", id);
        }
    }

    auto SceneManager::addModel(const Model* model) -> void {
        oneapi::tbb::spin_rw_mutex::scoped_lock lock(mRwMutex, true);
        mActiveScene->blass.emplace_back(model->as.blas);
        mActiveScene->meshBufferAddresses.emplace_back(model->mesh.meshBufAddress);
        mActiveScene->update = false;
        buildTLAS();
        mDevice.waitIdle();
        auto &bufAddr = mActiveScene->meshBufferAddressBuffer = mg::Allocator->allocBufferStaging(
                mActiveScene->meshBufferAddresses.size() * sizeof(sc::MeshBufAddress),
                mActiveScene->meshBufferAddresses.data(), vk::BufferUsageFlagBits::eStorageBuffer,
                "mesh buffer address");
        mRTDescriptor->updateDesSetAuto(mActiveScene->tlas, yic::ImageInfo{{}, mRTOffImage, vk::ImageLayout::eGeneral},
                                        globalCamera.getVpMatrixBuf(), bufAddr);
    }

    auto SceneManager::updateScene() -> void {

    }

    auto SceneManager::removeModel(const std::string &id) -> void {
//        auto rem = std::ranges::remove_if(mActiveScene->models, [&id](const Model* model){
//           return id == model->info.id;
//        });
//        mActiveScene->models.erase(rem.begin(), rem.end());

//        std::vector<size_t> remove;
//        for(size_t i = 0; i < mActiveScene->models.size(); i++){
//            if (id == mActiveScene->models[i]->info.id){
//                remove.emplace_back(i);
//            }
//        }
//
//        for(auto& it : std::ranges::reverse_view(remove)){
//            mActiveScene->models.erase(mActiveScene->models.begin() + static_cast<std::ptrdiff_t>(it));
//            mActiveScene->blass.erase(mActiveScene->blass.begin() + static_cast<std::ptrdiff_t>(it));
//        }
    }

    auto SceneManager::cBlas(Model* model) -> void {
        auto& m = model->mesh;
        INIT_MULTI(vk::DeviceOrHostAddressConstKHR,
                   vertBufAddr{mg::Allocator->getDeviceAddress(m.vertBuf)},
                   indexBufAddr{mg::Allocator->getDeviceAddress(m.indexBuf)});

        auto maxVert = (uint32_t )m.vertices.size();
        auto numTri = (uint32_t )m.indices.size() / 3;

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometryType(vk::GeometryTypeKHR::eTriangles)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                                     .setTriangles(vk::AccelerationStructureGeometryTrianglesDataKHR()
                                                           .setVertexFormat(vk::Format::eR32G32B32Sfloat)
                                                           .setVertexData(vertBufAddr)
                                                           .setVertexStride(sizeof(sc::Vertex))
                                                           .setMaxVertex(maxVert)
                                                           .setIndexType(vk::IndexType::eUint32)
                                                           .setIndexData(indexBufAddr)));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTri, mDyDispatcher);

        auto& blas = model->as.blas = mg::Allocator->allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eBottomLevel);

        auto scratchBuf = mg::Allocator->allocBufferStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer, "blas scratch buf");
        auto scratchBufAddr = vk::DeviceOrHostAddressKHR{mg::Allocator->getDeviceAddress(scratchBuf)};

        asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(blas->accel)
                .setScratchData(scratchBufAddr);

        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setFirstVertex(0)
                .setPrimitiveOffset(0)
                .setPrimitiveCount(numTri)
                .setTransformOffset(0);

        yic::CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
            cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, mDyDispatcher);
            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                      vk::AccessFlagBits::eAccelerationStructureReadKHR};
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                {}, barrier, {}, {});
        });
    }

    auto SceneManager::buildTLAS() -> void {
        auto tfMat = mg::Allocator->glmMatToVkTransformMatrix();
        auto& blass = mActiveScene->blass;

        vot::vector<vk::AccelerationStructureInstanceKHR> asInsts(blass.size());
        for(auto i = 0; i < blass.size(); i++){
            asInsts[i].setTransform(tfMat)
                    .setInstanceCustomIndex(0)
                    .setMask(0xff)
                    .setInstanceShaderBindingTableRecordOffset(0)
                    .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                    .setAccelerationStructureReference(mg::Allocator->getDeviceAddress(blass[i]));
        }

        auto instBuf = mg::Allocator->allocBufferStaging(sizeof(vk::AccelerationStructureInstanceKHR) * blass.size(), asInsts.data(),
                                                         vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                         vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR, "tlas insts buf");

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setGeometryType(vk::GeometryTypeKHR::eInstances)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setInstances(vk::AccelerationStructureGeometryInstancesDataKHR()
                .setArrayOfPointers(vk::False)
                .setData(mg::Allocator->getBufAddr(instBuf))));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, blass.size(), mDyDispatcher);

        auto& tlas = mActiveScene->tlas = mg::Allocator->allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eTopLevel);

        auto scratchBuf = mg::Allocator->allocBufferStaging(asBuildSizeInfo.accelerationStructureSize,
                                                            vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer, "tlas scratch buf");

        asBuildGeomInfo.setMode(mActiveScene->update ? vk::BuildAccelerationStructureModeKHR::eUpdate
                                                     : vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(tlas->accel)
                .setScratchData(vk::DeviceOrHostAddressKHR{mg::Allocator->getDeviceAddress(scratchBuf)});

        auto asBuildRangInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setPrimitiveCount(blass.size())
                .setTransformOffset(0)
                .setPrimitiveOffset(0)
                .setFirstVertex(0);

        yic::CommandBufferCoordinator::buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangInfo);
    }

    auto SceneManager::cRTRenderGroup() -> void {
        mRayTracingGroup = yic::RenderGroupRayTracing ::configure()
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addDesSetLayout_(0, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR)
                ->addDesSetLayout_(0, 2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR)
                ->addDesSetLayout_(0, 3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addShader_("b_rt_gen.rgen", vk::ShaderStageFlagBits::eRaygenKHR)
                ->addShader_("b_rt_shadow.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("b_rt_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("b_rt_hit_buffer_ref.rchit", vk::ShaderStageFlagBits::eClosestHitKHR)
                ->bindDescriptor(mRTDescriptor)
                ->build();
    }

    auto SceneManager::render() -> void {
        mRenderHandle->updateDescriptor(PrimaryRenderSeq::eRT, mRenderTargetOffImage);

        mRenderHandle->appendProcessCommand(PrimaryRenderSeq::eRT, [&](vk::CommandBuffer &cmd, vk::ImageSubresourceRange subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}) {
            if (mActiveScene != nullptr && mActiveScene->tlas != nullptr) {
                oneapi::tbb::spin_rw_mutex::scoped_lock lock(mRwMutex, false);

                cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, mRayTracingGroup->acquire());
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, mRayTracingGroup->getPipelineLayout(), 0,
                                       mRTDescriptor->getDescriptorSets(), {});
                cmd.traceRaysKHR(mRayTracingGroup->getRegionRgen(), mRayTracingGroup->getRegionMiss(),
                                 mRayTracingGroup->getRegionHit(), mRayTracingGroup->getRegionCall(),
                                 mExtent.width, mExtent.height, 1, mDyDispatcher);

                mg::Allocator->pipelineBarrier2(cmd, {},
                                                vk::ImageMemoryBarrier2()
                                                        .setImage(mRTOffImage->images)
                                                        .setOldLayout(vk::ImageLayout::eGeneral)
                                                        .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                                                        .setSrcAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
                                                        .setDstAccessMask(vk::AccessFlagBits2::eTransferRead)
                                                        .setSrcStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
                                                        .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                                                        .setSubresourceRange(subresourceRange),
                                                vk::ImageMemoryBarrier2()
                                                        .setImage(mRenderTargetOffImage->images)
                                                        .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                                                        .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                                                        .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                        .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
                                                        .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                        .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                                                        .setSubresourceRange(subresourceRange));
                mg::Allocator->copyImageToImage(mRTOffImage, mRenderTargetOffImage, cmd);
                mg::Allocator->pipelineBarrier2(cmd, {},
                                                vk::ImageMemoryBarrier2()
                                                        .setImage(mRTOffImage->images)
                                                        .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                                                        .setNewLayout(vk::ImageLayout::eGeneral)
                                                        .setSrcAccessMask(vk::AccessFlagBits2::eTransferRead)
                                                        .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
                                                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                                                        .setDstStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
                                                        .setSubresourceRange(subresourceRange),
                                                vk::ImageMemoryBarrier2()
                                                        .setImage(mRenderTargetOffImage->images)
                                                        .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                                                        .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                                                        .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                                                        .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                        .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                                                        .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                        .setSubresourceRange(subresourceRange));
            }
        });
    }





} // sc