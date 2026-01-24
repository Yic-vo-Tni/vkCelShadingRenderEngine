//
// Created by lenovo on 9/5/2025.
//

#include "ComputePipeline.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "Utils/FileOperation.h"

namespace rhi {

    ComputePipeline::ComputePipeline() : ct(yic::systemHub.va<ev::pVkSetupContext>()) {}

    ComputePipeline::~ComputePipeline() {
        ct.device->destroy(mPipeline);
        pdSetLayout.reset();
    }

    auto ComputePipeline::combine(const vot::gfx::api::ComputePipelineCI &createInfo) -> void {
        const auto sm = vot::gfx::detail::createShaderModule(ct.device, createInfo.shaderPath, vot::gfx::detail::ShaderType::eCompute);
        pdSetLayout = std::make_shared<vot::gfx::PipelineDescriptorSetLayout>(createInfo.pdSetLayoutCI);

        const auto stageInfo = vk::PipelineShaderStageCreateInfo()
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setModule(sm)
                .setPName("main");

        const auto ci = vk::ComputePipelineCreateInfo()
                .setStage(stageInfo)
                .setLayout(pdSetLayout->vaPipelineLayout());
        mPipeline = ct.device->createComputePipeline(nullptr, ci).value;

        ct.device->destroy(sm);
    }

    auto ComputePipeline::dispatch(const vot::CommandBuffer &cmd, const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) const -> void {
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pdSetLayout->vaPipelineLayout(), DS.startIndex, DS.va(), {});
        cmd.dispatch(x, y, z);
    }
} // rhi