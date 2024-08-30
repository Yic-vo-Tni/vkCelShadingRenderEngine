//
// Created by lenovo on 5/23/2024.
//

#include "vkRhi.h"

namespace yic {

    vkRhi::vkRhi() {
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelF{vk::True};
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtF{vk::True};
        vk::PhysicalDeviceBufferDeviceAddressFeatures devAddr{vk::True};
        vk::PhysicalDeviceShaderAtomicInt64Features shaderInt64F{vk::True};
        vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{vk::True};
//        auto rayTracingPipeline = vk::PhysicalDeviceRayTracingPipelineFeaturesKHR()
//                .setRayTracingPipeline(vk::True);
//        auto accelerationStructure = vk::PhysicalDeviceAccelerationStructureFeaturesKHR()
//                .setAccelerationStructure(vk::True);
//        auto bufferDeviceAddress = vk::PhysicalDeviceBufferDeviceAddressFeatures()
//                .setBufferDeviceAddress(vk::True);
//        auto shaderAtomicInt64 = vk::PhysicalDeviceShaderAtomicInt64Features()
//                .setShaderBufferInt64Atomics(vk::True);
//        auto descriptorIndexing = vk::PhysicalDeviceDescriptorIndexingFeatures()
//                .setShaderInputAttachmentArrayDynamicIndexing(vk::True)
//                .setShaderInputAttachmentArrayNonUniformIndexing(vk::True)
//                .setDescriptorBindingSampledImageUpdateAfterBind(vk::True)
//                .setDescriptorBindingVariableDescriptorCount(vk::True)
//                .setDescriptorBindingPartiallyBound(vk::True);
        vkWindow::get();
        vkInit::get(std::make_shared<vkInitCreateInfo>()
                            ->addInstanceLayers("VK_LAYER_KHRONOS_validation")
                      //      ->addInstanceLayers("VK_LAYER_LUNARG_api_dump")
                            ->addInstanceExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelF)
                            ->addPhysicalExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &devAddr)
                            ->addPhysicalExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rtF)
                            ->addPhysicalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)

                            ->setQueuesPriority(std::vector<float>{1.f, 0.9f})
        );

        auto ct = EventBus::Get::vkSetupContext();
        {
            auto id = et::vkRenderContext::id::mainRender;
            auto queue = ct.qGraphicsPrimary_ref();
            auto qIndex = ct.qIndexGraphicsPrimary_v();

            mSwapchain = std::make_unique<vkSwapchain>(id, queue, qIndex);
            RenderProcessManager::get();
            Allocator::get();

            TaskBus::registerTask(tt::RenderTarget_s::eMainWindow, [this] {
                FrameLoop();
            });
        }

    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
        EventBus::Get::vkSetupContext().device_ref().waitIdle();

        Allocator::clear();
        RenderProcessManager::clear();
        ImGuiDescriptorManager::clear();
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
    }



} // yic