//
// Created by lenovo on 5/30/2024.
//

#include "vkSwapchain.h"

namespace yic {

    vkSwapchain::vkSwapchain(vk::Format format) : mDevice(EventBus::Get::vkDeviceContext().device.value()),
                                                  mPhysicalDevice(EventBus::Get::vkDeviceContext().physicalDevice.value()),
                                                  mSurface(EventBus::Get::vkInitContext().surface.value()),
                                                  mGraphicsQueueFamilyIndex(EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsFamilyIndex()),

                                                  mSurfaceFormat([&] {
                                                      auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
                                                      for (auto &i: formats) {
                                                          if (i.format == format)
                                                              return i;
                                                      }
                                                      return formats[0];
                                                  }()),

                                                  mSwapchain(createSwapchain({})) {

        EventBus::publish(et::vkSwapchainContext{mSwapchain, mImageViews});

        TaskBus::registerTask(tt::RebuildSwapchain::eSwapchainRebuild, [&]() {
            auto oldSwapchain = mSwapchain;
            auto newSwapchain = createSwapchain(oldSwapchain);

            if (newSwapchain != VK_NULL_HANDLE) {
                setSwapchain(newSwapchain);
            } else { std::cerr << "failed to create new swapchain" << "\n"; }
        });
    }

    vkSwapchain::~vkSwapchain() {
        if (mSwapchain)
            mDevice.destroy(mSwapchain);
    }

    auto vkSwapchain::createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR {
        auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
        auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);
        auto extent = EventBus::Get::vkWindowContext().extent.value();

        mExtent = capabilities.currentExtent.width == (uint32_t) - 1 ? extent : capabilities.currentExtent;
        mImageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount,
                                 capabilities.maxImageCount ? capabilities.maxImageCount : UINT32_MAX);

        auto presentMode = [&] {
            using mode = vk::PresentModeKHR;

            for (const auto &m: {mode::eMailbox, mode::eImmediate}) {
                if (std::ranges::find_if(presentModes, [&m](const mode& mode) {return mode == m;}) != presentModes.end())
                    return m;
            }
            return mode::eFifo;
        }();

        Rvk_t("create swapchain", spdlog::level::warn) = [&]() {
            return mDevice.createSwapchainKHR(vk::SwapchainCreateInfoKHR{{},
                                                                         mSurface, mImageCount, mSurfaceFormat.format,
                                                                         mSurfaceFormat.colorSpace,
                                                                         mExtent, 1,
                                                                         vk::ImageUsageFlagBits::eColorAttachment,
                                                                         vk::SharingMode::eExclusive,
                                                                         mGraphicsQueueFamilyIndex,
                                                                         vk::SurfaceTransformFlagBitsKHR::eIdentity,
                                                                         vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                                         presentMode, vk::True, oldSwapchain});
        };
    }

} // yic