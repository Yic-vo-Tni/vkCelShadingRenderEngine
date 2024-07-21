//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessManager.h"

namespace yic {

    RenderProcessManager::RenderProcessManager() {
        vk::Extent2D et{1920, 1080};

        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)] = std::make_shared<vkRenderProcess>(et, enum_name(RenderProcessPhases::ePrimary));

        mCameraBuf = vkAllocator::allocBuf(sizeof (glm::mat4), vk::BufferUsageFlagBits::eUniformBuffer, vkAllocator::MemoryUsage::eCpuToGpu, "camera");
        mDescriptor = vkAllocator::allocDesc(IdGenerator::uniqueId());
        mDescriptor->addDesSetLayout({
            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
        });
        mPipeline = std::make_shared<GraphicsPipeline>(mDescriptor->getPipelineLayout(), vkFrameRender::eColorDepthStencilRenderPass);
        mPipeline->rasterizationState.setCullMode(vk::CullModeFlagBits::eNone);
        mPipeline->addShader("v_test.spv", vk::ShaderStageFlagBits::eVertex)
                .addShader("f_test.spv", vk::ShaderStageFlagBits::eFragment)
                .create();
        mDescriptor->updateDesSet({vkDescriptor::BufInfo{mCameraBuf->buffer}});

        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)]->appendCommandRecord([&](vk::CommandBuffer& cmd){
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline->Get());
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mDescriptor->getPipelineLayout(), 0, mDescriptor->getDescriptorSets(),
                                   nullptr);
            cmd.draw(3, 1, 0, 0);
        });
    }

    RenderProcessManager::~RenderProcessManager() = default;

    auto RenderProcessManager::RenderProcedure() -> std::vector<vk::CommandBuffer> {
        auto& cmds = get()->cmds;

        cmds.clear();

        for(auto& key : {RenderProcessPhases::ePrimary}){
            auto it = get()->mRenderProcess.find(enum_name(key));
            if (it != get()->mRenderProcess.end()){
                cmds.emplace_back(it->second->process());
            }
        }

        return {cmds.begin(), cmds.end()};
    }

    auto RenderProcessManager::prepare() -> void {
        sc::globalCamera.computeViewProjMatrix();
        get()->mCameraBuf->updateBuf(sc::globalCamera.getVpMatrix());

        tbb::parallel_for_each(get()->mRenderProcess.begin(), get()->mRenderProcess.end(), [](auto& it){
            if (it.second){
                it.second->prepare();
            }
        });
    }

} // yic