//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_VKEVENTTYPES_H
#define VKCELSHADINGRENDERER_VKEVENTTYPES_H

namespace et {

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
                 (opt<std::vector<vk::ImageView>>, imageViews));
    };

    struct vkRenderPassContext {
        struct_y(vkRenderPassContext,
                 (opt<vk::RenderPass>, renderPass));
    };

}

#endif //VKCELSHADINGRENDERER_VKEVENTTYPES_H
