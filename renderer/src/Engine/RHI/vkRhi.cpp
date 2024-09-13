//
// Created by lenovo on 5/23/2024.
//

#include "vkRhi.h"

namespace yic {

    vkRhi::vkRhi() {
//        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelF{vk::True};
//        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rtF{vk::True};
//        vk::PhysicalDeviceBufferDeviceAddressFeatures devAddr{vk::True};
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
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKhr{vk::True};
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKhr{vk::True};
        vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeaturesKhr{vk::True};
        vk::PhysicalDeviceSynchronization2FeaturesKHR synchronization2FeaturesKhr{vk::True};

        vkWindow::get(1920, 1080);
        vkInit::get(std::make_shared<vkInitCreateInfo>()
                            ->addInstanceLayers("VK_LAYER_KHRONOS_validation")
                                    //      ->addInstanceLayers("VK_LAYER_LUNARG_api_dump")
//                            ->addInstanceExtensions(VK_KHR_display)
//                            ->addInstanceExtensions(VK_KHR_external_fence_capabilities)
                            ->addInstanceExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelerationStructureFeaturesKhr)
                            ->addPhysicalExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &bufferDeviceAddressFeaturesKhr)
                            ->addPhysicalExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rayTracingPipelineFeaturesKhr)
                            ->addPhysicalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
//                            ->addPhysicalExtensions(VK_EXT_subpass_merge_feedback)
                            ->addPhysicalExtensions(VK_NV_FRAMEBUFFER_MIXED_SAMPLES_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)
                                    // render
//                            ->addPhysicalExtensions(VK_EXT_mesh_shader)
//                            ->addPhysicalExtensions(VK_EXT_robustness2)
//                            ->addPhysicalExtensions(VK_EXT_transform_feedback)
//                                    // compute
//                            ->addPhysicalExtensions(VK_EXT_shader_demote_to_helper_invocation)
//                                    // video
//                            ->addPhysicalExtensions(VK_KHR_video_queue)
//                            ->addPhysicalExtensions(VK_KHR_video_decode_queue)
//                                    // other
//                            ->addPhysicalExtensions(VK_KHR_timeline_semaphore)
                            ->addPhysicalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, &synchronization2FeaturesKhr)

                            ->setQueuesPriority(std::vector<float>{1.f, 0.9f})
        );

        EventBus::update(et::ResolutionRatio{vk::Extent2D{2560, 1440}});

        mg::Allocator = Allocator::make();
        {
            auto qf = mg::SystemHub.val<ev::pVkSetupContext>().queueFamily;
            mSwapchain = std::make_unique<vkSwapchain>(toolkit::enum_name(RenderPhase::ePrimary), qf->gPrimary(), qf->gIndexPrimary());
            mRenderProcessHandler = std::make_unique<RenderProcessHandler>();
        }
        mg::SceneManager = sc::SceneManager::make();

    }

    vkRhi::~vkRhi() {
        mg::SystemHub.val<ev::pVkSetupContext>().queueFamily->gPrimary().waitIdle();
        mg::SystemHub.val<ev::pVkSetupContext>().device->waitIdle();

        mg::Allocator->clear();
        mRenderProcessHandler->clear();
        ImGuiDescriptorManager::clear();
        EventBus::destroy<et::vkResource>();
    }

    bool vkRhi::FrameLoop() {
        beginFrame();

//        ShaderHotReLoader::executeShaderTask();
        mg::SystemHub.process();
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