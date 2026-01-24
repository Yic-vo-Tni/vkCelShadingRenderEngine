//
// Created by lenovo on 1/21/2026.
//

#ifndef VKCELSHADINGRENDERER_PIPELINEDESCRIPTORSETLAYOUT_H
#define VKCELSHADINGRENDERER_PIPELINEDESCRIPTORSETLAYOUT_H

namespace vot::gfx {
    class PipelineDescriptorSetLayout {
    public:
        explicit PipelineDescriptorSetLayout(const api::PipelineDescriptorSetLayoutCI& createInfo);
        ~PipelineDescriptorSetLayout();

        auto vaPipelineLayout() const { return pipelineLayout; }
    private:
        auto build() -> void;
    private:
        ev::pVkSetupContext ct;
        api::PipelineDescriptorSetLayoutCI pdSetLayoutCI;
        vk::PipelineLayout pipelineLayout;
        container::vector<vk::DescriptorSetLayout> setLayouts;
    };

}

#endif //VKCELSHADINGRENDERER_PIPELINEDESCRIPTORSETLAYOUT_H
