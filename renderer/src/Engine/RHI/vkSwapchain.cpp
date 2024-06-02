//
// Created by lenovo on 5/30/2024.
//

#include "vkSwapchain.h"

namespace yic {

    vkSwapchain::vkSwapchain(vk::Format format) :
        mDevice                     (EventBus::Get::vkDeviceContext().device),
        mPhysicalDevice             (EventBus::Get::vkDeviceContext().physicalDevice),
        mSurface                    (EventBus::Get::vkInitContext().surface),
        mGraphicsQueueFamilyIndex   (EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsFamilyIndex().value()),

        mSurfaceFormat([&]{
            auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
            for (auto &i: formats) {
                if (i.format == format) {
                    return i;
                }
            }
            return formats[0];
        }()),

        mSwapchain(createSwapchain({}))
    {

    }

    vkSwapchain::~vkSwapchain() {
        mDevice.destroy(mSwapchain);
    }

    auto vkSwapchain::createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR {
        return vkCreate("create swapchain") = [&](){

            auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
            auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

            mExtent = [&]{
                if (capabilities.currentExtent.width == (uint32_t ) - 1){
                    return vk::Extent2D{};
                }

                return capabilities.currentExtent;
            }();
            mImageCount = [&] -> uint32_t {
                return std::clamp(capabilities.minImageCount + 1,
                                  capabilities.minImageCount,
                                  capabilities.maxImageCount ? capabilities.maxImageCount : UINT32_MAX);
            }();

            auto presentMode = [&]{
                using mode = vk::PresentModeKHR;

                for(const auto& m : {mode::eMailbox, mode::eImmediate}){
                    auto found = std::ranges::find_if(presentModes, [&m](const mode& mode){
                        return mode == m;
                    });
                    if (found != presentModes.end()){
                        return *found;
                    };
                }
                return mode::eFifo;
            }();


            return mDevice.createSwapchainKHR(vk::SwapchainCreateInfoKHR{{},
                                                                         mSurface, mImageCount, mSurfaceFormat.format, mSurfaceFormat.colorSpace,
                                                                         mExtent, 1, vk::ImageUsageFlagBits::eColorAttachment,
                                                                         vk::SharingMode::eExclusive, mGraphicsQueueFamilyIndex,
                                                                         vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                                         presentMode, vk::True, oldSwapchain}
                    );
        };
    }

    bool vkSwapchain::update(const vk::Extent2D &extent) {
        auto oldSwapchain = mSwapchain;

        auto createSwapchain = [&,
                                chooseExtent = [&]{
           auto capabilities =  mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
           if (capabilities.currentExtent.width == (uint32_t) - 1){
               return vk::Extent2D{};
           } else{
               return capabilities.currentExtent;
           }
        }, choosePresentMode = [&]{
            auto presentMode = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

            using mode = vk::PresentModeKHR;
            auto preMode = mode::eFifo;

            for(auto& m : presentMode){
                if (m == mode ::eMailbox){
                    preMode = mode ::eMailbox;
                    break;
                }
                if (m == mode::eImmediate){
                    preMode = mode ::eImmediate;
                }
            }
        }](){

        };

        return true;
    }

} // yic