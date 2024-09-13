//
// Created by lenovo on 5/30/2024.
//

#ifndef VKCELSHADINGRENDERER_VKSWAPCHAIN_H
#define VKCELSHADINGRENDERER_VKSWAPCHAIN_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

#include "Engine/RHI/RenderSession.h"
#include "Editor/Gui/vkImGui.h"

namespace yic {

    class vkSwapchain : nonCopyable{
    public:
        explicit vkSwapchain(const vot::string& id,
                             vk::Queue graphicsQueue, const uint32_t& queueFamilyIndex,
                             vk::Format format = vk::Format::eR8G8B8A8Unorm);
        ~vkSwapchain();

        auto updateEveryFrame() -> void;
        auto submitFrame(const std::vector<vk::CommandBuffer>& cmds = {}, const std::function<void()>& fun = {}) -> void;
    private:
        auto createSurface() -> vk::SurfaceKHR;
        auto createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR;
        auto createFrameEntries() -> std::vector<et::FrameEntry>;
        auto createRenderPass() -> vk::RenderPass;
        auto createFrameBuffers() -> std::vector<vk::Framebuffer>;
        auto createFences() -> std::vector<vk::Fence>;
        auto acquire() -> bool;
        auto setSwapchain(vk::SwapchainKHR swapchainKhr) -> void{
            if (mSwapchain != swapchainKhr){
                if (mSwapchain)
                    mDevice.destroy(mSwapchain);
                mSwapchain = swapchainKhr;
            }
        };
        auto recreateSwapchain() -> void;
        auto chooseSurfaceFormat(vk::Format format) -> vk::SurfaceFormatKHR;
        auto destroyResource() -> void;

    private:
        GLFWwindow* mWindow{};
        vk::Instance mInstance{};
        vk::Device mDevice{};
        vk::PhysicalDevice mPhysicalDevice{};
        vk::SurfaceKHR mSurface{};
        vk::Queue mGraphicsQueue{};

    private:
        std::string Id;
        vot::string mId;
        vk::SurfaceFormatKHR mSurfaceFormat;
        vk::Extent2D mExtent{};
        vk::SwapchainKHR mSwapchain{};
        std::unique_ptr<RenderSession> mRenderSession;
        std::unique_ptr<vkImGui> mImGui;
        uint32_t mGraphicsQueueFamilyIndex{UINT32_MAX};
        uint32_t mImageCount{UINT32_MAX};
        uint32_t mImageIndex{UINT32_MAX};
        uint32_t mCurrentFrame{0};
        std::vector<et::FrameEntry> mFrameEntries{};
        vk::RenderPass mRenderPass{};
        std::vector<vk::Framebuffer> mFramebuffers{};
        std::vector<vk::Fence> mFences{};
        std::atomic<bool> mUpdateSize{false};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKSWAPCHAIN_H
