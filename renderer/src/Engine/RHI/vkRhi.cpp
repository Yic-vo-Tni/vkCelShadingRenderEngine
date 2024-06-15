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

        TaskBus::registerTask(tt::EngineFlow::eRhi, [this]{ run();});
    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsQueue().waitIdle();
        EventBus::Get::vkDeviceContext().device.value().waitIdle();
    }

    bool vkRhi::run() {
       // SemaphoreGuard guard{mRun_semaphore};

//        if (!mContinueRunning.load(std::memory_order_acquire)){
//            return false;
//        }

        mSwapchain->updateEveryFrame();
        auto cmd = mCommand->beginCommandBuf();

        mCommand->endCommandBuf(cmd);
        std::vector<vk::CommandBuffer> cmds{cmd};
        mSwapchain->submitFrame(cmds);


        return true;
    }




} // yic