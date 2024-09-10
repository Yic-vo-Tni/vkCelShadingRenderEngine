//
// Created by lenovo on 8/11/2024.
//

#include "RT.h"

namespace yic {


    auto RT::build::blas(const vk::AccelerationStructureGeometryKHR& asGeom, const uint32_t& numTri) -> void {
//        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
//                .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
//                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
//                .setGeometries(asGeom);
//
//        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, numTri, mDispatcher);
//        auto x = mDevice;
//
//        auto blas = Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eBottomLevel);
//
//        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
//        auto scratchBufAddr = vk::DeviceOrHostAddressKHR{Allocator::getBufAddr(scratchBuf)};
//
//        asBuildGeomInfo.setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
//                .setDstAccelerationStructure(blas->accel)
//                .setScratchData(scratchBufAddr);
//
//        auto asBuildRangeInfo = vk::AccelerationStructureBuildRangeInfoKHR()
//                .setFirstVertex(0)
//                .setPrimitiveOffset(0)
//                .setPrimitiveCount(numTri)
//                .setTransformOffset(0);
//
//        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
//            cmd.buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangeInfo, mDispatcher);
//            vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
//                                      vk::AccessFlagBits::eAccelerationStructureReadKHR};
//            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
//                                vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
//                                {}, barrier, {}, {});
//        });
    }

    auto RT::build::tlas(const std::vector<vkAccel_sptr>& blass, vkAccel_sptr& tlas, bool update) -> void{
//        auto tfMat = Allocator::glmMatToVkTransformMatrix();
//
//        std::vector<vk::AccelerationStructureInstanceKHR> insts(blass.size());
//        for(const auto& blas : blass){
//            insts.emplace_back(vk::AccelerationStructureInstanceKHR()
//                                       .setTransform(tfMat)
//                                       .setInstanceCustomIndex(0)
//                                       .setMask(0xFF)
//                                       .setInstanceShaderBindingTableRecordOffset(0)
//                                       .setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable)
//                                       .setAccelerationStructureReference(Allocator::getAccelDevAddr(blas)));
//        }
//
//        auto instBuf = Allocator::allocBufStaging(sizeof(vk::AccelerationStructureInstanceKHR) * insts.size(), insts.data(),
//                                                  vk::BufferUsageFlagBits::eShaderDeviceAddress |
//                                                  vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);
//
//        auto asGeom = vk::AccelerationStructureGeometryKHR()
//                .setGeometryType(vk::GeometryTypeKHR::eInstances)
//                .setFlags(vk::GeometryFlagBitsKHR::eOpaque)
//                .setGeometry(vk::AccelerationStructureGeometryDataKHR()
//                .setInstances(vk::AccelerationStructureGeometryInstancesDataKHR()
//                .setArrayOfPointers(vk::False)
//                .setData(Allocator::getBufAddr(instBuf))));
//        auto asBuildGeomInfo = vk::AccelerationStructureBuildGeometryInfoKHR()
//                .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
//                .setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
//                .setGeometries(asGeom);
//
//        auto asBuildSizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, insts.size(), mDispatcher);
//
//        if (!update)
//            tlas = Allocator::allocAccel(asBuildSizeInfo, vk::AccelerationStructureTypeKHR::eTopLevel);
//
//        auto scratchBuf = Allocator::allocBufStaging(asBuildSizeInfo.accelerationStructureSize,
//                                                     vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eStorageBuffer);
//
//        asBuildGeomInfo.setMode(!update ? vk::BuildAccelerationStructureModeKHR::eBuild : vk::BuildAccelerationStructureModeKHR::eUpdate)
//                .setDstAccelerationStructure(tlas->accel)
//                .setScratchData(vk::DeviceOrHostAddressKHR{Allocator::getBufAddr(scratchBuf)});
//
//        auto asBuildRangInfo = vk::AccelerationStructureBuildRangeInfoKHR()
//                .setPrimitiveCount(1)
//                .setTransformOffset(0)
//                .setPrimitiveOffset(0)
//                .setFirstVertex(0);
//
//        CommandBufferCoordinator::buildAccelerationStructuresKHR(asBuildGeomInfo, &asBuildRangInfo);
    }

} // yic