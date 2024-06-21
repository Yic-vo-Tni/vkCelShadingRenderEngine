//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKEVENTTYPES_H
#define VKCELSHADINGRENDERER_VKEVENTTYPES_H

namespace et {

    struct FrameEntry {
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };

    struct vkSetupContext {
        HANA(vkSetupContext,
             (opt<vk::Instance>, instance),
             (opt<vk::DispatchLoaderDynamic>, dynamicDispatcher),
             (opt<vk::DebugUtilsMessengerEXT>, debugMessenger),
             (opt<vk::PhysicalDevice>, physicalDevice),
             (opt<vk::Device>, device),
             (opt<yic::QueueFamily>, queueFamily));


        RETURN_REF(instance);
        RETURN_REF(dynamicDispatcher);
        RETURN_REF(debugMessenger);
        RETURN_REF(physicalDevice);
        RETURN_REF(device);

        RETURN_CUSTOM_REF(queue_imGuiGraphics, queueFamily, queueFamily->getImGuiGraphicsQueue());
        RETURN_CUSTOM_REF(queue_mainGraphics, queueFamily, queueFamily->getMainGraphicsQueue());

        RETURN_CUSTOM_VALUE(qIndex_imGuiGraphics, queueFamily, queueFamily->getImGuiGraphicsFamilyIndex());
        RETURN_CUSTOM_VALUE(qIndex_mainGraphics, queueFamily, queueFamily->getMainGraphicsFamilyIndex());
    };

    struct vkPipeline {
        HANA(vkPipeline,
             (opt < std::unordered_map<std::string, vk::Pipeline>>, graphics),
             (opt < std::unordered_map<std::string, vk::Pipeline>>, compute),
             (opt < std::unordered_map<std::string, vk::Pipeline>>, raytracing),
             (opt < vk::DescriptorSet > , descriptorSet));
    };


    struct vkRenderContext {
        HANA(vkRenderContext,
             (opt<std::pair<int, int>>, size),
             (opt<vk::Extent2D>, extent),
             (opt<GLFWwindow*>, window),
             (opt<vk::SwapchainKHR>, swapchain),
             (opt<vk::Extent2D>, currentExtent),
             (opt<std::vector<FrameEntry>>, frameEntries),
             (opt<vk::SurfaceFormatKHR>, surfaceFormat),
             (opt<uint32_t>, activeImageIndex),
             (opt<vk::RenderPass>, renderPass),
             (opt<std::vector<vk::Framebuffer>>, framebuffers),
             (opt<std::unordered_map<std::string, vk::RenderPass>>, offscreenRenderPass),
             (opt<std::unordered_map<std::string, std::vector<vk::Framebuffer>>>, offscreenFramebuffers),
             (opt<vk::CommandBuffer>, cmd)
        );

        struct id {
            static constexpr const char *imguiRender{"imguiRender"};
        };

        RETURN_REF(window);
        RETURN_REF(swapchain);
        RETURN_REF(frameEntries);
        RETURN_REF(surfaceFormat);
        RETURN_REF(renderPass);
        RETURN_REF(framebuffers);
        RETURN_REF(cmd);

        RETURN_VALUE(extent);
        RETURN_VALUE(currentExtent);
        RETURN_VALUE(activeImageIndex);

        RETURN_CUSTOM_VALUE(width, size, size->first);
        RETURN_CUSTOM_VALUE(height, size, size->second);
        RETURN_CUSTOM_VALUE(imageCount, frameEntries, static_cast<uint32_t>(frameEntries->size()));

        RETURN_MAP_REF(offscreenRenderPass, std::string);
        RETURN_MAP_REF(offscreenFramebuffers, std::string);

    };


    struct test {
        HANA(test,
             (opt < int > , x));

        RETURN_REF(x);
    };



}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
