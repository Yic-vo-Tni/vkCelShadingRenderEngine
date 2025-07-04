//
// Created by lenovo on 9/24/2024.
//

#include "Core/DispatchSystem/SystemHub.h"
#include "Rhi.h"
#include "QueueFamily.h"
#include "TimelineSemaphore.h"
#include "Command.h"
#include "Allocator.h"
#include "Descriptor.h"
#include "DescriptorSystem.h"

namespace rhi {
    Rhi::Rhi() {
        mFrameRate = std::make_unique<FrameRate>();

        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeaturesKhr{vk::True};
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeaturesKhr{vk::True};
        vk::PhysicalDevicePipelineExecutablePropertiesFeaturesKHR pipelineExecutablePropertiesFeaturesKhr{vk::True};
        vk::PhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeaturesKhr{vk::True};
        vk::PhysicalDeviceSynchronization2FeaturesKHR synchronization2FeaturesKhr{vk::True};
        vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR timelineSemaphoreFeaturesKhr{vk::True};
        vk::PhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeaturesKhr{vk::True};
        vk::PhysicalDeviceMultiviewFeaturesKHR multiviewFeatures{};
        vk::PhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{};
        vk::PhysicalDeviceDescriptorBufferFeaturesEXT descriptorBufferFeaturesExt{vk::True, vk::True};
        vk::PhysicalDeviceTransformFeedbackFeaturesEXT transformFeedbackFeaturesExt{};
        vk::PhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeaturesExt{};
        vk::PhysicalDeviceGraphicsPipelineLibraryFeaturesEXT graphicsPipelineLibraryFeaturesExt{vk::True};
        vk::PhysicalDeviceShaderModuleIdentifierFeaturesEXT shaderModuleIdentifierFeaturesExt{};
        vk::PhysicalDeviceShaderObjectFeaturesEXT shaderObjectFeaturesExt{};
        vk::PhysicalDeviceDynamicRenderingLocalReadFeaturesKHR dynamicRenderingLocalReadFeaturesKhr{vk::True};
        vk::PhysicalDeviceDeviceGeneratedCommandsFeaturesNV deviceGeneratedCommandsFeaturesNv{};
        vk::PhysicalDeviceMeshShaderFeaturesEXT meshShaderFeaturesExt{vk::True, vk::True};
        vk::PhysicalDeviceRobustness2FeaturesEXT robustness2FeaturesExt{vk::True, vk::True};
//        vk::PhysicalDeviceHostImageCopyFeaturesEXT hostImageCopyFeaturesExt{};

        mVkInit = std::make_unique<VkInit>(VkInit::CreateInfo()
                .addInstanceLayers("VK_LAYER_KHRONOS_validation")
                .addInstanceExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)

                .addPhysicalFeatures(vk::PhysicalDeviceFeatures2KHR().features
                                             .setShaderInt64(vk::True)
                                             .setSamplerAnisotropy(vk::True)
                                             .setGeometryShader(vk::True)
                                             .setRobustBufferAccess(vk::True)
                                             .setTessellationShader(vk::True))
                .addPhysicalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_SPIRV_1_4_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME)
                .addPhysicalExtensions(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME)
                .addPhysicalExtensions(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, &rayTracingPipelineFeaturesKhr)
                .addPhysicalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelerationStructureFeaturesKhr)
                .addPhysicalExtensions(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, &bufferDeviceAddressFeaturesKhr)
                .addPhysicalExtensions(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, &synchronization2FeaturesKhr)
                .addPhysicalExtensions(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, &timelineSemaphoreFeaturesKhr)
                .addPhysicalExtensions(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, &dynamicRenderingFeaturesKhr)
                .addPhysicalExtensions(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME, &pipelineExecutablePropertiesFeaturesKhr)
                .addPhysicalExtensions(VK_EXT_SHADER_OBJECT_EXTENSION_NAME, &shaderObjectFeaturesExt)
                .addPhysicalExtensions(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, &descriptorIndexingFeatures)
                .addPhysicalExtensions(VK_EXT_DESCRIPTOR_BUFFER_EXTENSION_NAME, &descriptorBufferFeaturesExt)
                .addPhysicalExtensions(VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME, &transformFeedbackFeaturesExt)
                .addPhysicalExtensions(VK_EXT_CONDITIONAL_RENDERING_EXTENSION_NAME, &conditionalRenderingFeaturesExt)
                .addPhysicalExtensions(VK_EXT_GRAPHICS_PIPELINE_LIBRARY_EXTENSION_NAME, &graphicsPipelineLibraryFeaturesExt)
                .addPhysicalExtensions(VK_EXT_SHADER_MODULE_IDENTIFIER_EXTENSION_NAME, &shaderModuleIdentifierFeaturesExt)
                .addPhysicalExtensions(VK_NVX_MULTIVIEW_PER_VIEW_ATTRIBUTES_EXTENSION_NAME, &multiviewFeatures)
                .addPhysicalExtensions(VK_NV_DEVICE_GENERATED_COMMANDS_EXTENSION_NAME, &deviceGeneratedCommandsFeaturesNv)
                .addPhysicalExtensions(VK_EXT_MESH_SHADER_EXTENSION_NAME, &meshShaderFeaturesExt)
                .addPhysicalExtensions(VK_KHR_DYNAMIC_RENDERING_LOCAL_READ_EXTENSION_NAME, &dynamicRenderingLocalReadFeaturesKhr)
                .addPhysicalExtensions(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, &robustness2FeaturesExt)

                // not support
//                .addPhysicalExtensions(VK_EXT_HOST_IMAGE_COPY_EXTENSION_NAME, &hostImageCopyFeaturesExt)

                .setQueuesPriority({1.f, 0.8f}));

        mSwapchain = std::make_unique<Swapchain>();

        yic::command = rhi::CommandManager::make();
        yic::timeline = rhi::TimelineSemaphore::make();
        yic::allocator = rhi::Allocator::make();
        yic::desSystem = rhi::DescriptorSystem::make();
        yic::imguiImage = rhi::ImGuiDescriptorManager::make();
    }

    Rhi::~Rhi() {
        if_debug yic::logger->warn("~ rhi");
        mSwapchain->clear();
        yic::allocator->clear();
        yic::timeline->clear();
        yic::command->clear();
        yic::desSystem->clear();
        yic::imguiImage->clear();
        mSwapchain.reset();
        mVkInit.reset();
    }

    auto Rhi::render() -> void {
        mSwapchain->draw();
        mFrameRate->update();
    }

} // rhi