//
// Created by lenovo on 9/5/2025.
//

#include "ComputePipeline.h"

#include <utility>

#include "Core/DispatchSystem/SystemHub.h"
#include "Utils/FileOperation.h"

namespace rhi {

    ComputePipeline::ComputePipeline() {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
    }

    ComputePipeline::~ComputePipeline() {
        ct.device->destroy(mPipeline);
        ct.device->destroy(mPipelineLayout);
        mDesSetLayoutCI.clear(ct.device);
    }

    auto ComputePipeline::addShader(vot::string path) const -> vk::ShaderModule {
        path = spv_path + path + ".spv";
        vot::vector<char> v;
        std::ranges::copy(fo::loadFile(path), std::back_inserter(v));
        return ct.device->createShaderModule(vk::ShaderModuleCreateInfo()
                                                              .setCodeSize(v.size() * sizeof(char))
                                                              .setPCode(reinterpret_cast<const uint32_t *>(v.data())));
    }

    auto ComputePipeline::build(const vot::string& pt, vot::PipelineDescriptorSetLayoutCI2 descriptor_set_layout_ci2) -> ComputePipeline & {
        const auto sm = addShader(pt);
        mDesSetLayoutCI = std::move(descriptor_set_layout_ci2);
        mPipelineLayout = mDesSetLayoutCI.buildPipelineSetLayout(ct.device);
        yic::logger->warn(mDesSetLayoutCI.desSetLayouts.size());
        yic::logger->warn(mDesSetLayoutCI.setLayoutBindings[1].size());

        const auto stageInfo = vk::PipelineShaderStageCreateInfo()
                .setStage(vk::ShaderStageFlagBits::eCompute)
                .setModule(sm)
                .setPName("main");

        const auto ci = vk::ComputePipelineCreateInfo()
                .setStage(stageInfo)
                .setLayout(mPipelineLayout);
        mPipeline = ct.device->createComputePipeline(nullptr, ci).value;

        ct.device->destroy(sm);

        return *this;
    }


    auto ComputePipeline::dispatch(const vot::CommandBuffer &cmd, const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) const -> void {
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mPipelineLayout, DS.startIndex, DS.va(), {});
        cmd.dispatch(x, y, z);
    }



} // rhi