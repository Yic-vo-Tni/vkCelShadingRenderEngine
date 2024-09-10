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
        vkWindow::get(1920, 1080);
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

        EventBus::update(et::ResolutionRatio{vk::Extent2D{2560, 1440}});
        {
            mg::Allocator = Allocator::make();
        }

        auto ct = mg::SystemHub.val<ev::pVkSetupContext>();
        {
            auto id = et::vkRenderContext::id::mainRender;

            mSwapchain = std::make_unique<vkSwapchain>(id, ct.queueFamily->gPrimary(), ct.queueFamily->gIndexPrimary());
            mRenderProcessHandler = std::make_unique<RenderProcessHandler>();
        }


    }

    vkRhi::~vkRhi() {
        mg::SystemHub.val<ev::pVkSetupContext>().queueFamily->gPrimary().waitIdle();
        mg::SystemHub.val<ev::pVkSetupContext>().device->waitIdle();

        //Allocator::clear();
        mg::Allocator->clear();
        mRenderProcessHandler->clear();
        ImGuiDescriptorManager::clear();
        EventBus::destroy<et::vkResource>();
    }

    bool vkRhi::FrameLoop() {
        beginFrame();

//        ShaderHotReLoader::executeShaderTask();
        mRenderProcessHandler->prepare();
        mSwapchain->updateEveryFrame();

        auto cmds = mRenderProcessHandler->procedure();
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