//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKEVENTTYPES_H
#define VKCELSHADINGRENDERER_VKEVENTTYPES_H

namespace et {

    struct FrameEntry{
        vk::Image image;
        vk::ImageView imageView;
        vk::Semaphore readSemaphore;
        vk::Semaphore writtenSemaphore;
    };

    struct vkInitContext {
        struct_y(vkInitContext,
                 (opt<vk::Instance>, instance),
                 (opt<vk::DispatchLoaderDynamic>, dynamicDispatcher),
                 (opt<vk::DebugUtilsMessengerEXT>, debugMessenger),
                 (opt<vk::SurfaceKHR>, surface));
    };

    struct vkDeviceContext {
        struct_y(vkDeviceContext,
                 (opt<vk::PhysicalDevice>, physicalDevice),
                 (opt<vk::Device>, device),
                 (opt<yic::QueueFamily>, queueFamily));
    };

    struct vkSwapchainContext {
        struct_y(vkSwapchainContext,
                 (opt<vk::SwapchainKHR>, swapchain),
                 (opt<std::vector<FrameEntry>>, frameEntries),
                 (opt<vk::SurfaceFormatKHR>, surfaceFormat),
                 (opt<uint32_t>, activeImageIndex));
    };

    struct vkFrameRenderContext {
        struct_y(vkFrameRenderContext,
                 (opt<vk::RenderPass>, renderPass),
                 (opt<std::vector<vk::Framebuffer>>, framebuffers));

        struct id{
            static constexpr const char* imguiFrameRender{"imguiFrameRender"};
        };
    };

    struct vkCommandBufContext{
        struct_y(vkCommandBufContext,
                 (opt<vk::CommandBuffer>, cmd));
    };

}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
