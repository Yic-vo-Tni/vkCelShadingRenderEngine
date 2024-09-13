//
// Created by lenovo on 5/30/2024.
//

#include "vkSwapchain.h"

namespace yic {

    vkSwapchain::vkSwapchain(const vot::string &id, vk::Queue graphicsQueue, const uint32_t &queueFamilyIndex,
                             vk::Format format) :
            mId(id),
            mWindow(mg::SystemHub.val<ev::hVkRenderContext>(id).window),
            mInstance(*mg::SystemHub.val<ev::pVkSetupContext>().instance),
            mDevice(*mg::SystemHub.val<ev::pVkSetupContext>().device),
            mPhysicalDevice(*mg::SystemHub.val<ev::pVkSetupContext>().physicalDevice),
            mSurface(createSurface()),
            mGraphicsQueue(graphicsQueue),
            mGraphicsQueueFamilyIndex(queueFamilyIndex),
            mSurfaceFormat(chooseSurfaceFormat(format)),
            mSwapchain(createSwapchain({})),
            mFrameEntries(createFrameEntries()),
            mRenderPass(createRenderPass()),
            mFramebuffers(createFrameBuffers()),
            mFences(createFences()){

        mg::SystemHub.sto(ev::hVkRenderContext{
            .swapchain = &mSwapchain,
            .frameEntries = &mFrameEntries,
            .surfaceFormat = &mSurfaceFormat,
            .activeImageIndex = &mImageIndex,
            .renderPass = &mRenderPass,
            .framebuffers = &mFramebuffers,
        }, mId);


        mRenderSession = std::make_unique<RenderSession>(mId.c_str(), queueFamilyIndex, mImageCount);
        mImGui = std::make_unique<vkImGui>(mId, graphicsQueue, queueFamilyIndex);

        mg::SystemHub.subscribe([&](const ev::oWindowSizeChange& change){
            mGraphicsQueue.waitIdle();
            recreateSwapchain();
            mCurrentFrame = 0;
        });
    }

    vkSwapchain::~vkSwapchain() {
        for(auto& f : mFences){
            mDevice.destroy(f);
        }
        mDevice.destroy(mRenderPass);
        destroyResource();
        if (mSwapchain)
            mDevice.destroy(mSwapchain);
        mInstance.destroy(mSurface);
    }

    auto vkSwapchain::createSurface() -> vk::SurfaceKHR {
        VkSurfaceKHR tempSurface{};

        if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &tempSurface) == VK_SUCCESS) {
            return tempSurface;
        } else {
            throw std::runtime_error("failed to create glfw surface");
        }

        Rvk_y("create surface") = [&] {
            return tempSurface;
        };
    }

    auto vkSwapchain::chooseSurfaceFormat(vk::Format format) -> vk::SurfaceFormatKHR {
        auto formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
        for (auto &i: formats) {
            if (i.format == format)
                return i;
        }
        return formats[0];
    }

    auto vkSwapchain::createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR {
        auto capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
        auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);
        auto extent = mg::SystemHub.valEvent<ev::oWindowSizeChange>().extent.value();

        mExtent = capabilities.currentExtent.width == (uint32_t) - 1 ? extent : capabilities.currentExtent;
        mg::SystemHub.sto(ev::hVkRenderContext{.currentExtent = &mExtent}, mId);
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
        if (!acquire())
            throw std::runtime_error("failed to acquire swap chain image");

        if (mDevice.waitForFences(mFences[mImageIndex], VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
            throw std::runtime_error("failed to wait fence\n");
    }

    auto vkSwapchain::submitFrame(const std::vector<vk::CommandBuffer>& cmds, const std::function<void()>& fun) -> void {
        auto& cmd = mRenderSession->beginCommandBuf(mExtent);
        mRenderSession->beginRenderPass(RenderSession::passInfo{
                mRenderPass, mFramebuffers, mExtent,
                RenderSession::clearValue::color
        });

        mImGui->render(cmd);

        mRenderSession->endRenderPass();
        mRenderSession->endCommandBuf();

        mDevice.resetFences(mFences[mImageIndex]);

        auto waitStage = std::vector<vk::PipelineStageFlags>{vk::PipelineStageFlagBits::eColorAttachmentOutput};

        auto i = mCurrentFrame % mImageCount;
        auto finalCmds = cmds;
        finalCmds.emplace_back(cmd);
        vk::SubmitInfo info{mFrameEntries[i].readSemaphore, waitStage, finalCmds, mFrameEntries[i].writtenSemaphore};

        mGraphicsQueue.submit(info, mFences[mImageIndex]);
        vk::PresentInfoKHR presentInfoKhr{mFrameEntries[i].writtenSemaphore, mSwapchain, mCurrentFrame};

        auto r = mGraphicsQueue.presentKHR(&presentInfoKhr);
        if (r == vk::Result::eSuccess) {
        } else {
            mGraphicsQueue.waitIdle();
            recreateSwapchain();
            auto semaphore = mDevice.createSemaphore(vk::SemaphoreCreateInfo());
            auto rv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);
            switch (rv.result) {
                case vk::Result::eSuccess:
                    mImageIndex = rv.value;
                    break;
                default:
                    mDevice.destroy(semaphore);
                    return;
            }
            mDevice.destroy(semaphore);
        }

        mCurrentFrame = (mCurrentFrame + 1) % mImageCount;
    }

    auto vkSwapchain::acquire() -> bool {
        auto semaphore = mFrameEntries[mCurrentFrame % mImageCount].readSemaphore;

        auto rv = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE);
        switch (rv.result) {
            case vk::Result::eSuccess:
                mImageIndex = rv.value;
                return true;
            case vk::Result::eSuboptimalKHR:
            case vk::Result::eErrorOutOfDateKHR:
                mGraphicsQueue.waitIdle();
                recreateSwapchain();
                mCurrentFrame = 0;
                return true;
            default:
                return false;
        }

    }

    auto vkSwapchain::recreateSwapchain() -> void {
        destroyResource();

        auto oldSwapchain = mSwapchain;
        auto newSwapchain = createSwapchain(oldSwapchain);

        setSwapchain(newSwapchain);
        mFrameEntries = createFrameEntries();
        mFramebuffers = createFrameBuffers();
    }

    auto vkSwapchain::createRenderPass() -> vk::RenderPass {
        std::vector<vk::AttachmentReference> attachRef{{0, vk::ImageLayout::eColorAttachmentOptimal}};
        std::vector<vk::AttachmentDescription> attachDes{
                {{},
                 mSurfaceFormat.format, vk::SampleCountFlagBits::e1,
                 vk::AttachmentLoadOp::eClear,
                 vk::AttachmentStoreOp::eStore,
                 vk::AttachmentLoadOp::eDontCare,
                 vk::AttachmentStoreOp::eDontCare,
                 vk::ImageLayout::eUndefined,
                 vk::ImageLayout::ePresentSrcKHR}
        };
        std::vector<vk::SubpassDescription> subpass{
                {
                        {}, vk::PipelineBindPoint::eGraphics, {},
                        attachRef[0], {}}
        };

        vk::RenderPassCreateInfo createInfo{{}, attachDes, subpass, {}};

        Rvk_t("create render pass", spdlog::level::warn) = [&] {
            return mDevice.createRenderPass(createInfo);
        };
    }

    auto vkSwapchain::createFrameBuffers() -> std::vector<vk::Framebuffer> {
        std::vector<vk::Framebuffer> framebuffers;
        framebuffers.resize(mImageCount);

        Rvk_y("create frame buffer") = [&]{
            for(size_t i = 0; i < mFrameEntries.size(); i++){
                auto view = mFrameEntries[i].imageView;

                auto createInfo = vk::FramebufferCreateInfo().setRenderPass(mRenderPass)
                        .setAttachments(view)
                        .setWidth(mExtent.width)
                        .setHeight(mExtent.height)
                        .setLayers(1);

                framebuffers[i] = mDevice.createFramebuffer(createInfo);
            }

            return framebuffers;
        };
    }

    auto vkSwapchain::destroyResource() -> void {
        for(auto& fb : mFramebuffers){
            mDevice.destroy(fb);
        }

        for(auto& f : mFrameEntries){
            mDevice.destroy(f.readSemaphore);
            mDevice.destroy(f.writtenSemaphore);
            mDevice.destroy(f.imageView);
        }
    }


} // yic