//
// Created by lenovo on 5/30/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSWAPCHAIN_H
#define VKCELSHADINGRENDERER_VKSWAPCHAIN_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkSwapchain : nonCopyable{
    public:
        explicit vkSwapchain(vk::Format format = vk::Format::eR8G8B8A8Unorm);
        ~vkSwapchain();

    private:
        auto createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR;
        auto setSwapchain(vk::SwapchainKHR swapchainKhr) -> void{
            if (mSwapchain != swapchainKhr){
                if (mSwapchain)
                    mDevice.destroy(mSwapchain);
                mSwapchain = swapchainKhr;
                EventBus::publish(et::vkSwapchainContext{.swapchain = mSwapchain});
            }
        };

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
        std::vector<vk::ImageView> mImageViews{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSWAPCHAIN_H
