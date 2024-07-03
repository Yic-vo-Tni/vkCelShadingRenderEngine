//
// Created by lenovo on 5/23/2024.
//

#include "vkRhi.h"

namespace yic {

    vkRhi::vkRhi() {
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelF{};
        vkWindow::get();
        vkInit::get(std::make_shared<vkInitCreateInfo>()
                            ->addInstanceLayers("VK_LAYER_KHRONOS_validation")
                            ->addInstanceExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelF)

                            ->setQueuesPriority(std::vector<float>{1.f, 0.9f})
        );

        auto ct = EventBus::Get::vkSetupContext();
        {
            auto id = et::vkRenderContext::id::mainRender;
            auto queue = ct.qGraphicsPrimary_ref();
            auto qIndex = ct.qIndexGraphicsPrimary_v();

            mImGuiSwapchain = std::make_unique<vkSwapchain>(id, queue, qIndex);
            mImGuiCommand = std::make_unique<vkCommand>(id, qIndex);
            mImGui = std::make_unique<vkImGui>(id, queue, qIndex);

            TaskBus::registerTask(tt::RenderTarget_s::eImGuiWindow, [this] {
                ImGuiFrameLoop();
            });
        }

        auto info = vk::PipelineLayoutCreateInfo();
        layout = EventBus::Get::vkSetupContext().device_ref().createPipelineLayout(info);
        pipeline = std::make_unique<vkPipeline<Graphics>>(EventBus::Get::vkSetupContext().device_ref(), layout,
                                      EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).renderPass_ref());
        pipeline->addShader(spv_path "/v_test.spv", vk::ShaderStageFlagBits::eVertex)
                .addShader(spv_path "/f_test.spv", vk::ShaderStageFlagBits::eFragment)
                .create();

        VkDeviceSize size;
        std::vector<int> t(5);
        auto buf = vkAllocator::allocBuf(sizeof (int) * t.size(), vk::BufferUsageFlagBits::eVertexBuffer,VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU);
        t.emplace_back(1);
        buf->updateBuf(t);

        mTimePerFrame = sf::seconds(1.f / 120.f);
    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkSetupContext().device_ref().destroy(layout);

        EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
        EventBus::Get::vkSetupContext().device_ref().waitIdle();
    }

    bool vkRhi::ImGuiFrameLoop() {
        sf::Time start = mClock.getElapsedTime();
//        sf::Time elapsed = mClock.restart();
//        while (elapsed < mTimePerFrame){
//            sf::sleep(mTimePerFrame - elapsed);
//            elapsed = mClock.getElapsedTime();
//        }

        TaskBus::executeShaderTask();

        mImGuiSwapchain->updateEveryFrame();
        mImGuiCommand->beginCommandBuf();

        auto cmd = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).cmd_ref();
        vk::Viewport viewport{
                0.f, 0.f,
                (float )EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).width_v(),
                (float )EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).height_v(),
                0.f, 1.f
        };
        cmd.setViewport(0, viewport);
        vk::Rect2D scissor{
                {0, 0},
                EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).extent_v()
        };
        cmd.setScissor(0, scissor);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Get());
        cmd.draw(3, 1, 0, 0);

        mImGui->render();

        mImGuiCommand->endCommandBuf();
        mImGuiSwapchain->submitFrame();

        sf::Time frameTime = mClock.getElapsedTime() - start;
        if (frameTime < mTimePerFrame){
            sf::sleep(mTimePerFrame - frameTime);
        }

        return true;
    }





} // yic