//
// Created by lenovo on 9/25/2024.
//

#ifndef VKCELSHADINGRENDERER_SWAPCHAIN_H
#define VKCELSHADINGRENDERER_SWAPCHAIN_H

#include "Editor/ImGui.h"

namespace rhi {

    class Swapchain {
    public:
        Swapchain();
        ~Swapchain();

        auto draw() -> void;

        [[nodiscard]] auto createSurface() const -> vk::SurfaceKHR;
        auto chooseSurfaceFormat(vk::Format format) -> vk::SurfaceFormatKHR;
        auto createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR;
        auto createFrameEntries() -> vot::vector<vot::FrameEntry>;
        auto prepareFrame() -> void;
        auto clear() -> void { mImGuiLauncher.reset();}
    private:
        ev::pVkSetupContext ct{};

        uint8_t mImageCount{};
        uint32_t mImageIndex{};
        uint8_t mCurrentFrame{};
        uint32_t mGraphicsQueueIndex{};
        vk::Extent2D mExtent{};
        vk::SurfaceKHR mSurface{};
        vk::SurfaceFormatKHR mSurfaceFormat{};
        vk::SwapchainKHR mSwapchain{};
        vot::FrameEntry* mFrameEntry{};
        vot::vector<vot::FrameEntry> mFrameEntries{};

        std::unique_ptr<ui::ImGuiLauncher> mImGuiLauncher;

        //
//        vot::CommandBuffer* Icmd = nullptr;
        vot::RHandle mRHandle = nullptr;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_SWAPCHAIN_H
