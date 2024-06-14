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

        auto updateEveryFrame() -> void;
        auto submitFrame(std::vector<vk::CommandBuffer>& cmd) -> void;
    private:
        auto createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR;
        auto createFrameEntries() -> std::vector<et::FrameEntry>;
        auto createFences() -> std::vector<vk::Fence>;
        auto acquire() -> bool;
        auto setSwapchain(vk::SwapchainKHR swapchainKhr) -> void{
            if (mSwapchain != swapchainKhr){
                if (mSwapchain)
                    mDevice.destroy(mSwapchain);
                mSwapchain = swapchainKhr;
                EventBus::publish(et::vkSwapchainContext{.swapchain = mSwapchain});
            }
        };

        auto destroyResource() -> void;

    private:
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::SurfaceKHR mSurface{};
        vk::Queue mGraphicsQueue{};

    private:
        vk::SurfaceFormatKHR mSurfaceFormat;
        vk::Extent2D mExtent{};
        vk::SwapchainKHR mSwapchain{};
        uint32_t mGraphicsQueueFamilyIndex{UINT32_MAX};
        uint32_t mImageCount{UINT32_MAX};
        uint32_t mImageIndex{UINT32_MAX};
        uint32_t mCurrentFrame{0};
        std::vector<et::FrameEntry> mFrameEntries{};
        std::vector<vk::Fence> mFences{};
        std::atomic<bool> mUpdateSize{false};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSWAPCHAIN_H
