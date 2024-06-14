//
// Created by lenovo on 5/30/2024.
//

#include "vkSwapchain.h"

namespace yic {

    vkSwapchain::vkSwapchain(vk::Format format) : mDevice(EventBus::Get::vkDeviceContext().device.value()),
                                                  mPhysicalDevice(EventBus::Get::vkDeviceContext().physicalDevice.value()),
                                                  mSurface(EventBus::Get::vkInitContext().surface.value()),
                                                  mGraphicsQueue(EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsQueue()),
                                                  mGraphicsQueueFamilyIndex(EventBus::Get::vkDeviceContext().queueFamily->getPrimaryGraphicsFamilyIndex()),
                                                  mSurfaceFormat([&] {
                                                      auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
                                                      for (auto &i: formats) {
                                                          if (i.format == format)
                                                              return i;
                                                      }
                                                      return formats[0];
                                                  }()),
                                                  mSwapchain(createSwapchain({})),
                                                  mFrameEntries(createFrameEntries()),
                                                  mFences(createFences()){

        EventBus::publish(et::vkSwapchainContext{mSwapchain, mFrameEntries, mSurfaceFormat});

        TaskBus::registerTask(tt::RebuildSwapchain::eSwapchainRebuild, [&]() {
            destroyResource();

            auto oldSwapchain = mSwapchain;
            auto newSwapchain = createSwapchain(oldSwapchain);

            setSwapchain(newSwapchain);
            mFrameEntries = createFrameEntries();

            EventBus::publish(et::vkSwapchainContext{.swapchain = mSwapchain, .frameEntries = mFrameEntries});
        });
    }

    vkSwapchain::~vkSwapchain() {
        for(auto& f : mFences){
            mDevice.destroy(f);
        }
        destroyResource();
        if (mSwapchain)
            mDevice.destroy(mSwapchain);
    }

    auto vkSwapchain::createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR {
        auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
        auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);
        auto extent = EventBus::Get::vkWindowContext().extent.value();

        mExtent = capabilities.currentExtent.width == (uint32_t) - 1 ? extent : capabilities.currentExtent;
        auto imageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount,
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
                                                                         mSurface, imageCount, mSurfaceFormat.format,
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

    auto vkSwapchain::createFrameEntries() -> std::vector<et::FrameEntry> {
        std::vector<et::FrameEntry> frameEntries;
        auto images = mDevice.getSwapchainImagesKHR(mSwapchain);
        mImageCount = images.size();
        frameEntries.resize(mImageCount);

        for(size_t i = 0; i < mImageCount; i++){
            auto& entry = frameEntries[i];

            entry.image = images[i];
            entry.imageView = mDevice.createImageView(vk::ImageViewCreateInfo{{},
                                                                              images[i], vk::ImageViewType::e2D,
                                                                              mSurfaceFormat.format,vk::ComponentSwizzle::eIdentity,
                                                                              vk::ImageSubresourceRange{ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});
            vkInfo("create image view " + std::to_string(i) + "successfully");

            auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
            entry.readSemaphore = mDevice.createSemaphore(semaphoreCreateInfo);
            entry.writtenSemaphore = mDevice.createSemaphore(semaphoreCreateInfo);
            vkTrance("create semaphore " + std::to_string(i) + "successfully");
        }
        return frameEntries;
    }

    auto vkSwapchain::createFences() -> std::vector<vk::Fence> {
        std::vector<vk::Fence> fences{};
        fences.resize(mImageCount);

        Rvk_y("create fences") = [&]{
            for(auto& f : fences){
                f = mDevice.createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
            }
            return fences;
        };
    }

    auto vkSwapchain::updateEveryFrame() -> void {
        EventBus::subscribeAuto([&](const et::WindowContext& windowContext){
            mUpdateSize.exchange(true);
        });
        if (mUpdateSize.load()){
            mDevice.waitIdle();
            mGraphicsQueue.waitIdle();
            TaskBus::executeTask<tt::RebuildSwapchain>();
            mUpdateSize.exchange(false);
        }

        if (!acquire())
            throw std::runtime_error("failed to acquire swap chain image");

        if (mDevice.waitForFences(1, &mFences[mImageIndex], VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
            throw std::runtime_error("failed to wait fence\n");
    }

    auto vkSwapchain::submitFrame(std::vector<vk::CommandBuffer>& cmd) -> void {
        mDevice.resetFences(mFences[mImageIndex]);

        auto waitStage = std::vector<vk::PipelineStageFlags>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

        auto i = mCurrentFrame % mImageCount;
        //auto cmd = EventBus::Get::vkCommandContext().cmd.value();
        vk::SubmitInfo info{mFrameEntries[i].readSemaphore, waitStage, cmd, mFrameEntries[i].writtenSemaphore};

        mGraphicsQueue.submit(info, mFences[mImageIndex]);
        vk::PresentInfoKHR presentInfoKhr{mFrameEntries[i].writtenSemaphore, mSwapchain, mCurrentFrame};

//        if (mGraphicsQueue.presentKHR(presentInfoKhr) != vk::Result::eSuccess) {
//            throw std::runtime_error("failed to present image");
//        }


            auto r = mGraphicsQueue.presentKHR(&presentInfoKhr);
            if (r == vk::Result::eSuccess) {
            } else {
                mDevice.waitIdle();
                mGraphicsQueue.waitIdle();
                TaskBus::executeTask<tt::RebuildSwapchain>();
            }


        mCurrentFrame = (mCurrentFrame + 1) % mImageCount;
    }

    auto vkSwapchain::acquire() -> bool {
        auto semaphore = mFrameEntries[mCurrentFrame % mImageCount].readSemaphore;

        auto rv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);
        switch (rv.result) {
            case vk::Result::eSuccess:
                mImageIndex = rv.value;
                EventBus::publish(et::vkSwapchainContext{.activeImageIndex = mImageIndex});
                return true;
            case vk::Result::eSuboptimalKHR:
            case vk::Result::eErrorOutOfDateKHR:
                mDevice.waitIdle();
                mGraphicsQueue.waitIdle();
                TaskBus::executeTask<tt::RebuildSwapchain>();
                {
                    auto newRv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);
                    if (newRv.result == vk::Result::eSuccess) {
                        mImageIndex = rv.value;
                        EventBus::publish(et::vkSwapchainContext{.activeImageIndex = mImageIndex});
                    }
                }
                return true;
            default:
                return false;
        }

//        vk::ResultValue<uint32_t> rv{vk::Result::eSuccess, 0};
//        do {
//            rv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);
//            if (rv.result == vk::Result::eErrorOutOfDateKHR) {
//                TaskBus::executeTask<tt::RebuildSwapchain>();
//                vkInfo("Failed to rebuild swapchain");
//                continue;
//            } else if (rv.result == vk::Result::eSuboptimalKHR) {
//                vkTrance("Failed to rebuild swapchain");
//                continue;
//            } else if (rv.result != vk::Result::eSuccess) {
//                return false;
//                throw std::runtime_error("Failed to acquire next image: " + std::to_string(rv.value));
//            }
//        } while (rv.result != vk::Result::eSuccess);
//        mImageIndex = rv.value;
//        EventBus::publish(et::vkSwapchainContext{.activeImageIndex = mImageIndex});

        return true;


    }

    auto vkSwapchain::destroyResource() -> void {
        for(auto& f : mFrameEntries){
            mDevice.destroy(f.readSemaphore);
            mDevice.destroy(f.writtenSemaphore);
            mDevice.destroy(f.imageView);
        }
    }


} // yic