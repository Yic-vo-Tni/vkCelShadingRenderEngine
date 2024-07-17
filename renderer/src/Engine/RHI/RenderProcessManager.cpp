//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessManager.h"

namespace yic {

    RenderProcessManager::RenderProcessManager() {
        vk::Extent2D et{1920, 1080};

        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)] = std::make_shared<vkRenderProcess>(et, enum_name(RenderProcessPhases::ePrimary));

        mPipeline = std::make_shared<GraphicsPipeline>(vkFrameRender::eColorDepthStencilRenderPass);
        mPipeline->addShader("v_test.spv", vk::ShaderStageFlagBits::eVertex)
                .addShader("f_test.spv", vk::ShaderStageFlagBits::eFragment)
                .create();

        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)]->appendCommandRecord([&](vk::CommandBuffer& cmd){
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->Get());
            cmd.draw(3, 1, 0, 0);
        });
    }

    RenderProcessManager::~RenderProcessManager() = default;

    auto RenderProcessManager::RenderProcedure() -> std::vector<vk::CommandBuffer> {
        auto& cmds = get()->cmds;
        auto RenderProcess = [&](RenderProcessPhases phases){
            return get()->mRenderProcess[enum_name(RenderProcessPhases::ePrimary)];
        };
        cmds.clear();
        cmds.emplace_back(RenderProcess(RenderProcessPhases::ePrimary)->process());

        return {cmds.begin(), cmds.end()};
    }

} // yic