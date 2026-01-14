//
// Created by lenovo on 9/5/2025.
//

#ifndef VKCELSHADINGRENDERER_COMPUTEPIPELINE_H
#define VKCELSHADINGRENDERER_COMPUTEPIPELINE_H

namespace rhi {
    class ComputePipeline{
    public:
        vot::DescriptorHandle DS;
        ComputePipeline();
        ~ComputePipeline();

        auto addShader(vot::string path) const -> vk::ShaderModule;
        auto build(const vot::string& pt, vot::PipelineDescriptorSetLayoutCI2 descriptor_set_layout_ci2) -> ComputePipeline&;
        auto dispatch(const vot::CommandBuffer& cmd, std::uint32_t x, std::uint32_t y, std::uint32_t z) const -> void;
        auto combine(const vot::ComputePipelineCI& createInfo) -> void {
            build(createInfo.shaderPath, createInfo.descriptorSetLayoutCI2);
        };

        [[nodiscard]] auto acquire() const { return mPipeline; }
        auto acquirePipelineBindPoint() { return vk::PipelineBindPoint::eCompute; }
        auto acquirePipelineLayout() { return mPipelineLayout; }
    private:
        ev::pVkSetupContext ct{};
        vk::Pipeline mPipeline{};
        vk::PipelineLayout mPipelineLayout{};
        vot::PipelineDescriptorSetLayoutCI2 mDesSetLayoutCI;
    };
} // rhi

#endif //VKCELSHADINGRENDERER_COMPUTEPIPELINE_H