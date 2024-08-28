//
// Created by lenovo on 8/11/2024.
//

#ifndef VKCELSHADINGRENDERER_RTBUILDER_H
#define VKCELSHADINGRENDERER_RTBUILDER_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/Command.h"
#include "Engine/RHI/Allocator.h"

namespace yic {

    class RTBuilder {
        enum shaderGroupType{
            eGeneral, eClosestHit, eAnyHit, eIntersection
        };
    public:
        RTBuilder();
        ~RTBuilder();
        auto cDesSets(const vkBuf_sptr& bufSptr) -> void;
        auto draw(const vk::CommandBuffer &cmd) -> void;
    private:
        auto cBLAS() -> void;
        auto cTLAS() -> void;
        auto cRtPipe() -> void;
        auto cSbt() -> void;
    private:
        vk::Device mDevice;
        vk::PhysicalDevice mPhysicalDevice;
        vk::DispatchLoaderDynamic mDyDispatcher;
        vk::Extent2D mExtent;

        vkBuf_sptr vertBuf, indexBuf, tfMatrixBuf, rgenSbt, rmissSbt, rchitSbt;
        vkImg_sptr offRtImg;
        vkAccel_sptr blas, tlas;

        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR  rtProperties{};

        vk::DescriptorSetLayout desSetLayout;
        vk::PipelineLayout pipeLayout;
        vk::Pipeline rtPipeline;
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups{};

        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptorSets;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RTBUILDER_H
