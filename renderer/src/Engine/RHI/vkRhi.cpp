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

            mSwapchain = std::make_unique<vkSwapchain>(id, queue, qIndex);
            RenderProcessManager::get();

            TaskBus::registerTask(tt::RenderTarget_s::eMainWindow, [this] {
                FrameLoop();
            });
        }

        mTimePerFrame = sf::seconds(1.f / 120.f);
    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
        EventBus::Get::vkSetupContext().device_ref().waitIdle();

        RenderProcessManager::clear();
        EventBus::destroy<et::vkResource>();
    }

    bool vkRhi::FrameLoop() {
        beginFrame();

        ShaderHotReLoader::executeShaderTask();
        RenderProcessManager::prepare();
        mSwapchain->updateEveryFrame();

        auto cmds = RenderProcessManager::RenderProcedure();
        mSwapchain->submitFrame(cmds);

        endFrame();

        return true;
    }

    auto vkRhi::beginFrame() -> void {
        mStart = mClock.getElapsedTime();
    }

    auto vkRhi::endFrame() -> void {
        mFrameTime = mClock.getElapsedTime() - mStart;
        EventBus::update(et::frameTime{
                .frameTime = mFrameTime.asSeconds()
        });
        if (mFrameTime < mTimePerFrame)
            sf::sleep(mTimePerFrame - mFrameTime);

    }


} // yic