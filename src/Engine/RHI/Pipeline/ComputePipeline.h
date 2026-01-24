//
// Created by lenovo on 9/5/2025.
//

#ifndef VKCELSHADINGRENDERER_COMPUTEPIPELINE_H
#define VKCELSHADINGRENDERER_COMPUTEPIPELINE_H

#include "PipelineTypes.h"
#include "RHI/Pipeline/PipelineDescriptorSetLayout.h"

namespace rhi {
    class ComputePipeline{
    public:
        vot::DescriptorHandle DS;
        ComputePipeline();
        ~ComputePipeline();

        auto combine(const vot::gfx::api::ComputePipelineCI& createInfo) -> void;
        auto dispatch(const vot::CommandBuffer& cmd, std::uint32_t x, std::uint32_t y, std::uint32_t z) const -> void;

        [[nodiscard]] auto acquire() const { return mPipeline; }
        constexpr auto acquirePipelineBindPoint() const noexcept { return vk::PipelineBindPoint::eCompute; }
        auto acquirePipelineLayout() const { return pdSetLayout->vaPipelineLayout(); }
    private:
        ev::pVkSetupContext ct{};
        vk::Pipeline mPipeline{};
        std::shared_ptr<vot::gfx::PipelineDescriptorSetLayout> pdSetLayout;
    };
} // rhi

#endif //VKCELSHADINGRENDERER_COMPUTEPIPELINE_H