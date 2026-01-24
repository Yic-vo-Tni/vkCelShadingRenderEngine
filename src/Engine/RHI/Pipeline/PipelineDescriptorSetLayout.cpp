//
// Created by lenovo on 1/21/2026.
//

#include "PipelineDescriptorSetLayout.h"

#include "Core/DispatchSystem/SystemHub.h"

namespace vot::gfx {
    PipelineDescriptorSetLayout::PipelineDescriptorSetLayout(const api::PipelineDescriptorSetLayoutCI &createInfo) : pdSetLayoutCI(createInfo), ct(yic::systemHub.va<ev::pVkSetupContext>()){
        build();
    }

    PipelineDescriptorSetLayout::~PipelineDescriptorSetLayout() {
        if (pipelineLayout)
            ct.device->destroy(pipelineLayout);

        for (const auto& layout : setLayouts) {
            ct.device->destroy(layout);
        }
        setLayouts.clear();
    }

    auto PipelineDescriptorSetLayout::build() -> void {
        pdSetLayoutCI.validate();

        for (auto& bindings : pdSetLayoutCI.setLayoutBindings | std::ranges::views::values) {
            auto ci = vk::DescriptorSetLayoutCreateInfo().setBindings(bindings);

            setLayouts.emplace_back(ct.device->createDescriptorSetLayout(ci));
        }

        const auto pipelineCI = vk::PipelineLayoutCreateInfo()
            .setPushConstantRanges(pdSetLayoutCI.pushConstantRanges)
            .setSetLayouts(setLayouts);

        pipelineLayout = ct.device->createPipelineLayout(pipelineCI);
    }

} // vot