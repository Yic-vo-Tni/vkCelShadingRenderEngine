//
// Created by lenovo on 7/25/2024.
//

#include "RenderGroup.h"

namespace yic {

    RenderGroup::RenderGroup(const vk::RenderPass &renderPass) :
            PipelineDesSetLayout(EventBus::Get::vkSetupContext().device_ref()),
            vkPipeline<Graphics>(renderPass) {

    }

    std::shared_ptr<RenderGroup> RenderGroup::addDesSetLayout_1(const uint32_t &set, const uint32_t &binding, const vk::DescriptorType &descriptorType,
                                 const uint32_t &descriptorCount, const vk::ShaderStageFlags &flags) {
        PipelineDesSetLayout::addDesSetLayout(set, binding, descriptorType, descriptorCount, flags);

        return shared_from_this();
    }

    std::shared_ptr<RenderGroup> RenderGroup::addPushConstantRange_2(const vk::ShaderStageFlags &flags, uint32_t offset, uint32_t size) {
        PipelineDesSetLayout::addPushConstantRange(flags, offset, size);

        return shared_from_this();
    }

    std::shared_ptr<RenderGroup> RenderGroup::addBindingDescription_3(const uint32_t &binding, const uint32_t &stride,
                                                    const vk::VertexInputRate &inputRate) {
        vkPipeline<Graphics>::addBindingDescription(binding, stride, inputRate);
        return shared_from_this();
    }

    std::shared_ptr<RenderGroup> RenderGroup::addAttributeDescription_4(const uint32_t &location, const uint32_t &binding, const vk::Format &format,
                                         const uint32_t &offset) {
        vkPipeline::addAttributeDescription(location, binding, format, offset);
        return shared_from_this();
    }

    std::shared_ptr<RenderGroup> RenderGroup::addShader_5(const std::string& path, vk::ShaderStageFlagBits flags) {
        Graphics::addShader(path, flags);
        return shared_from_this();
    }

    std::shared_ptr<RenderGroup> RenderGroup::build() {
        createInfo.setLayout(PipelineDesSetLayout::getPipelineLayout());
        Graphics::create();
        return shared_from_this();
    }


} // yic