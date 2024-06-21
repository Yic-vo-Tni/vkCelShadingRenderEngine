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
        // ImGui Render Pass
        {
            auto id = et::vkRenderContext::id::imguiRender;
            auto queue = ct.queue_imGuiGraphics_v();
            auto qIndex = ct.qIndex_imGuiGraphics_v();

            mImGuiSwapchain = std::make_unique<vkSwapchain>(id, queue, qIndex);
            mImGuiCommand = std::make_unique<vkCommand>(id, qIndex);
            mImGui = std::make_unique<vkImGui>(id, queue, qIndex);

            TaskBus::registerTask(tt::RenderTarget_s::eImGuiWindow, [this] {
                ImGuiFrameLoop();
            });
        }


    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkSetupContext().queue_mainGraphics_v().waitIdle();
        EventBus::Get::vkSetupContext().queue_imGuiGraphics_v().waitIdle();
        EventBus::Get::vkSetupContext().device_v().waitIdle();
    }

    bool vkRhi::ImGuiFrameLoop() {

        mImGuiSwapchain->updateEveryFrame();
        mImGuiCommand->beginCommandBuf();

        mImGui->render();

        mImGuiCommand->endCommandBuf();
        mImGuiSwapchain->submitFrame();

        return true;
    }





} // yic