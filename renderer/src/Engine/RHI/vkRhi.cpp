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

        mSwapchain = std::make_unique<vkSwapchain>();
        mFrameRender = std::make_unique<vkFrameRender>();
        mCommand = std::make_unique<vkCommand>();

        mImGui = std::make_unique<vkImGui>();

        TaskBus::registerTask(tt::EngineFlow::eRhi, [this]{ run();});
    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsQueue().waitIdle();
        EventBus::Get::vkDeviceContext().device.value().waitIdle();
    }

    bool vkRhi::run() {

        mSwapchain->updateEveryFrame();
        auto cmd = mCommand->beginCommandBuf();

        mImGui->beginRenderImGui();
        mImGui->endRenderImGui(cmd);

        mCommand->endCommandBuf(cmd);
        std::vector<vk::CommandBuffer> cmds{cmd};
        mSwapchain->submitFrame(cmds);


        return true;
    }




} // yic