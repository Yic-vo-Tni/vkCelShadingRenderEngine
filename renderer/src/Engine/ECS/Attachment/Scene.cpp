//
// Created by lenovo on 8/30/2024.
//

#include "Scene.h"

#include <ranges>
#include "Engine/RHI/Allocator.h"
#include "Engine/RHI/Command.h"

namespace sc {

    SceneManager::SceneManager() {
        yic::EventBus::valAuto(mDevice, mDyDispatcher);
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
        mActiveScene->models.emplace_back(model);
        mActiveScene->blass.emplace_back(cBlas(model));
        cTlas(mActiveScene->blass, mActiveScene->tlas);
    }

    auto SceneManager::updateScene() -> void {

    }

    auto SceneManager::removeModel(const std::string &id) -> void {
//        auto rem = std::ranges::remove_if(mActiveScene->models, [&id](const Model* model){
//           return id == model->info.id;
//        });
//        mActiveScene->models.erase(rem.begin(), rem.end());

        std::vector<size_t> remove;
        for(size_t i = 0; i < mActiveScene->models.size(); i++){
            if (id == mActiveScene->models[i]->info.id){
                remove.emplace_back(i);
            }
        }

        for(auto& it : std::ranges::reverse_view(remove)){
            mActiveScene->models.erase(mActiveScene->models.begin() + static_cast<std::ptrdiff_t>(it));
            mActiveScene->blass.erase(mActiveScene->blass.begin() + static_cast<std::ptrdiff_t>(it));
        }
    }

    auto SceneManager::cBlas(const Model* model) -> std::shared_ptr<yic::vkAccel> {
        auto& m = model->mesh;
        INIT_MULTI(vk::DeviceOrHostAddressConstKHR, vertBufAddr{yic::Allocator::getBufAddr(m.vertBuf)},
                   indexBufAddr{yic::Allocator::getBufAddr(m.indexBuf)});

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

        auto blas = yic::Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eBottomLevel);

        auto scratchBuf = yic::Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
        auto scratchBufAddr = vk::DeviceOrHostAddressKHR{yic::Allocator::getBufAddr(scratchBuf)};

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

        return std::move(blas);
    }

    auto SceneManager::cTlas(const std::vector<std::shared_ptr<yic::vkAccel>>& blass, std::shared_ptr<yic::vkAccel>& tlas) -> void {
        auto tfMat = yic::Allocator::glmMatToVkTransformMatrix();

        auto asInst = vk::AccelerationStructureInstanceKHR()
                .setTransform(tfMat)
                .setInstanceCustomIndex(0)
                .setMask(0xFF)
                .setInstanceShaderBindingTableRecordOffset(0)
                .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
                .setAccelerationStructureReference(yic::Allocator::getAccelDevAddr(blass.front()));

        auto instBuf = yic::Allocator::allocBufStaging(sizeof(vk::AccelerationStructureInstanceKHR), &asInst,
                                                  vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                  vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

        auto asGeom = vk::AccelerationStructureGeometryKHR()
                .setGeometryType(vk::GeometryTypeKHR::eInstances)
                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
                .setInstances(vk::AccelerationStructureGeometryInstancesDataKHR()
                .setArrayOfPointers(vk::False)
                .setData(yic::Allocator::getBufAddr(instBuf))));
        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
                .setGeometries(asGeom);

        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, 1, mDyDispatcher);

        if (tlas == nullptr){
            tlas = yic::Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eTopLevel);
        }

        auto scratchBuf = yic::Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize,
                                                     vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);

        asBuildGeomInfo.setMode(tlas->update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild)
                .setDstAccelerationStructure(tlas->accel)
                .setScratchData(vk::DeviceOrHostAddressKHR{yic::Allocator::getBufAddr(scratchBuf)});

        auto asBuildRangInfo = vk::AccelerationStructureBuildRangeInfoKHR()
                .setPrimitiveCount(1)
                .setTransformOffset(0)
                .setPrimitiveOffset(0)
                .setFirstVertex(0);

        yic::CommandBufferCoordinator::buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangInfo);
    }




} // sc