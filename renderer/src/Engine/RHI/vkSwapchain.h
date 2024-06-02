//
// Created by lenovo on 5/30/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSWAPCHAIN_H
#define VKCELSHADINGRENDERER_VKSWAPCHAIN_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/Event/Event.h"

namespace yic {

    class vkSwapchain : nonCopyable{
    public:
        explicit vkSwapchain(vk::Format format = vk::Format::eR8G8B8A8Unorm);
        ~vkSwapchain();

        bool update(const vk::Extent2D& extent2D);
    private:
        auto createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR;

    private:
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::SurfaceKHR mSurface{};

    private:
        vk::SurfaceFormatKHR mSurfaceFormat;
        vk::Extent2D mExtent{};
        vk::SwapchainKHR mSwapchain{};
        uint32_t mGraphicsQueueFamilyIndex{UINT32_MAX};
        uint32_t mImageCount{UINT32_MAX};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSWAPCHAIN_H
