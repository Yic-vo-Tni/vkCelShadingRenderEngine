//
// Created by lenovo on 8/11/2024.
//

#include "RT.h"

namespace yic {

//    auto RT::init() -> void {
//        auto ct = EventBus::Get::vkSetupContext();
//        mDevice = ct.device_ref();
//        mPhysicalDevice = ct.physicalDevice_ref();
//        mQueueIndex = ct.qIndexGraphicsPrimary_v();
//        mGraphicsQueue = ct.qGraphicsPrimary_ref();
//    }
//
//    auto RT::initRayTracing() -> void {
//        vk::PhysicalDeviceProperties2 properties2{};
//        properties2.pNext = &mRtProperties;
////        mPhysicalDevice.getProperties2(&properties2);
//    }
//
//    auto RT::createBLAS() -> void {
//        std::vector<RTBuilder::BLASInput> blas;
//        mRTBuilder.buildBLAS(blas, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
//    }
//
//    auto RT::createTLAS() -> void {
//        std::vector<vk::AccelerationStructureInstanceKHR> tlas;
//
//        vk::AccelerationStructureInstanceKHR asInst{
//                {}, 0, 0xFF, 0,
//                vk::GeometryInstanceFlagBitsKHR::eTriangleCullDisable, 0
//        };
//        mRTBuilder.buildTLAS(tlas, vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, false);
//    }

} // yic