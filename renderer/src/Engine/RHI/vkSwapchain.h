//
// Created by lenovo on 5/30/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSWAPCHAIN_H
#define VKCELSHADINGRENDERER_VKSWAPCHAIN_H

#include "Engine/Utils/Log.h"

namespace yic {

    class vkSwapchain : nonCopyable{
    public:
        explicit vkSwapchain(vk::Device device);
        ~vkSwapchain();


        bool update(const vk::Extent2D& extent2D);

    private:
        auto createSwapchain() -> vk::SwapchainKHR;

    private:
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::SurfaceKHR mSurface{};

    private:
        vk::SwapchainKHR mSwapchain{};
        vk::Extent2D mExtent{};
        uint32_t mGraphicsQueueFamilyIndex{UINT32_MAX};

        uint32_t mImageCount{UINT32_MAX};
        vk::Format mSurfaceFormat{};
        vk::ColorSpaceKHR mColorSpace{};

    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSWAPCHAIN_H
