//
// Created by lenovo on 9/25/2024.
//

#include "Swapchain.h"
#include "QueueFamily.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "TimelineSemaphore.h"
#include "Command.h"
#include "Allocator.h"

namespace rhi {
    Swapchain::Swapchain() : ct(yic::systemHub.val<ev::pVkSetupContext>()),
                             mSurface(createSurface()),
                             mSurfaceFormat(chooseSurfaceFormat(vk::Format::eR8G8B8A8Unorm)),
                             mSwapchain(createSwapchain(nullptr)),
                             mFrameEntries(createFrameEntries()){
        yic::systemHub.sto(ev::pVkRenderContext{
            .swapchain = &mSwapchain,
            .frameEntries = &mFrameEntries,
            .surfaceFormat = &mSurfaceFormat,
            .activeImageIndex = &mImageIndex,
        });

        mImGuiLauncher = std::make_unique<ui::ImGuiLauncher>();
    }

    Swapchain::~Swapchain() {
        for(auto& entry : mFrameEntries){
            ct.device->destroy(entry.writtenSemaphore);
            ct.device->destroy(entry.readSemaphore);
            ct.device->destroy(entry.imageView);
        }

        ct.device->destroy(mSwapchain);
        ct.instance->destroy(mSurface);
    }

    auto Swapchain::createSwapchain(vk::SwapchainKHR oldSwapchain) -> vk::SwapchainKHR {
        auto capabilities = ct.physicalDevice->getSurfaceCapabilitiesKHR(mSurface);
        auto presentModes = ct.physicalDevice->getSurfacePresentModesKHR(mSurface);
        auto extent = yic::systemHub.valEvent<ev::oWindowSizeChange>().extent.value_or(vk::Extent2D(0.f));

        mExtent = capabilities.currentExtent.width == (uint32_t) - 1 ? extent : capabilities.currentExtent;
        yic::systemHub.sto(ev::pVkRenderContext{.currentExtent = &mExtent});
        auto imageCount = std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount,
                                     capabilities.maxImageCount ? capabilities.maxImageCount : UINT32_MAX);

        auto presentMode = [&]{
            using mode = vk::PresentModeKHR;

            for(const auto& m : { mode::eMailbox, mode::eImmediate}){
                if (std::ranges::find_if(presentModes, [&m](const mode& mode){ return mode == m;}) != presentModes.end()){
                    return m;
                }
            }
            return mode::eFifo;
        }();

        return vot::create("create swapchain") = [&]{
            return ct.device->createSwapchainKHR(vk::SwapchainCreateInfoKHR{{},
                                                                            mSurface, imageCount,
                                                                            mSurfaceFormat.format,
                                                                            mSurfaceFormat.colorSpace,
                                                                            mExtent, 1,
                                                                            vk::ImageUsageFlagBits::eColorAttachment,
                                                                            vk::SharingMode::eExclusive,
                                                                            mGraphicsQueueIndex,
                                                                            vk::SurfaceTransformFlagBitsKHR::eIdentity,
                                                                            vk::CompositeAlphaFlagBitsKHR::eOpaque,
                                                                            presentMode, vk::True, oldSwapchain
            });
        };
    }

    auto Swapchain::createSurface() const -> vk::SurfaceKHR {
        VkSurfaceKHR temp{};

        if (glfwCreateWindowSurface(*ct.instance, yic::systemHub.val<ev::pWindowContext>().window, nullptr, &temp) == VK_SUCCESS){
            return temp;
        } else { throw std::runtime_error("failed to create glfw window");}
    }

    auto Swapchain::chooseSurfaceFormat(vk::Format format) -> vk::SurfaceFormatKHR {
        auto formats = ct.physicalDevice->getSurfaceFormatsKHR(mSurface);

        for(auto &i : formats){
            if (i.format == format)
                return i;
        }
        return formats[0];
    }

    auto Swapchain::createFrameEntries() -> vot::vector<vot::FrameEntry> {
        auto images = ct.device->getSwapchainImagesKHR(mSwapchain);
        mImageCount = images.size();

        vot::vector<vot::FrameEntry> frameEntries(mImageCount);
        for(auto i = 0; i < mImageCount; i++){
            auto& entry = frameEntries[i];

            entry.image = images[i];
            entry.imageView = ct.device->createImageView(vk::ImageViewCreateInfo{{},
                                                                                 images[i], vk::ImageViewType::e2D,
                                                                                 mSurfaceFormat.format, vk::ComponentSwizzle::eIdentity,
                                                                                 vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}});
            if_debug yic::logger->info("create image view " + std::to_string(i) + "successfully");

            auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
            entry.readSemaphore = ct.device->createSemaphore(semaphoreCreateInfo);
            entry.writtenSemaphore = ct.device->createSemaphore(semaphoreCreateInfo);
        }

        return frameEntries;
    }

    auto Swapchain::prepareFrame() -> void {
        mFrameEntry = &mFrameEntries[mCurrentFrame % mImageCount];

        auto r = ct.device->acquireNextImageKHR(mSwapchain, UINT64_MAX, mFrameEntry->readSemaphore, nullptr, &mImageIndex);

        if ((r == vk::Result::eErrorOutOfDateKHR) || (r == vk::Result::eSuboptimalKHR)){
            if (r == vk::Result::eErrorOutOfDateKHR){

            }
            return;
        }
    }

    auto Swapchain::draw() -> void {
//        auto& cmd = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);
//
//        prepareFrame();
//
//        auto& image = mFrameEntry->image;
//
//        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
//
//        yic::allocator->pipelineBarrier2(cmd, {},
//                                         vk::ImageMemoryBarrier2()
//                                                 .setImage(image)
//                                                 .setOldLayout(vk::ImageLayout::eUndefined)
//                                                 .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
//                                                 .setSrcAccessMask(vk::AccessFlagBits2::eNone)
//                                                 .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
//                                                 .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
//                                                 .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
//                                                 .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
//
//        auto colorAttach = vk::RenderingAttachmentInfo()
//                .setImageView(mFrameEntries[mImageIndex].imageView)
//                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
//                .setLoadOp(vk::AttachmentLoadOp::eClear)
//                .setStoreOp(vk::AttachmentStoreOp::eStore)
//                .setClearValue(vk::ClearValue{vk::ClearColorValue{1.f, 0.f, 0.f, 1.f}});
//
//        auto renderingInfo = vk::RenderingInfo()
//                .setRenderArea(vk::Rect2D{{0, 0}, mExtent})
//                .setLayerCount(1)
//                .setColorAttachments(colorAttach);
//
//        cmd.beginRendering(renderingInfo);
//
//        mImGuiLauncher->draw(cmd);
//
//        cmd.endRendering();
//
//        yic::allocator->pipelineBarrier2(cmd, {},
//                                         vk::ImageMemoryBarrier2()
//                                                 .setImage(image)
//                                                 .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
//                                                 .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
//                                                 .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
//                                                 .setDstAccessMask(vk::AccessFlagBits2::eNone)
//                                                 .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
//                                                 .setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
//                                                 .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
//
//        cmd.end();
//
//        yic::timeline->finalSubmit(mSwapchain, mImageIndex, vot::SubmitInfo()
//                .setCommandBuffers(cmd)
//                .setWaitValues({vot::timelineStage::ePrepare, vot::timelineStage::ePrepare})
//                .setSignalValues({vot::timelineStage::ePresent, vot::timelineStage::ePresent})
//                .setWaitSemaphore(mFrameEntry->readSemaphore)
//                .setSignalSemaphore(mFrameEntry->writtenSemaphore)
//                .setWaitStageMasks({vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eColorAttachmentOutput}));
//
//        mCurrentFrame = (mCurrentFrame + 1) % mImageCount;



       // if (mRHandle == nullptr) mRHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);

        yic::command->bind(yic::command->acquire(mRHandle, vot::threadSpecificCmdPool::eMainRender), [&](vot::CommandBuffer& cmd){
            prepareFrame();

            auto& image = mFrameEntry->image;

            cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

            yic::allocator->pipelineBarrier2(cmd, {},
                                             vk::ImageMemoryBarrier2()
                                                     .setImage(image)
                                                     .setOldLayout(vk::ImageLayout::eUndefined)
                                                     .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                     .setSrcAccessMask(vk::AccessFlagBits2::eNone)
                                                     .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                     .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                     .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                     .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

            auto colorAttach = vk::RenderingAttachmentInfo()
                    .setImageView(mFrameEntries[mImageIndex].imageView)
                    .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setClearValue(vk::ClearValue{vk::ClearColorValue{0.f, 0.f, 0.f, 1.f}});

            auto renderingInfo = vk::RenderingInfo()
                    .setRenderArea(vk::Rect2D{{0, 0}, mExtent})
                    .setLayerCount(1)
                    .setColorAttachments(colorAttach);

            cmd.beginRendering(renderingInfo);

            mImGuiLauncher->draw(cmd);

            cmd.endRendering();

            yic::allocator->pipelineBarrier2(cmd, {},
                                             vk::ImageMemoryBarrier2()
                                                     .setImage(image)
                                                     .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                     .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                                                     .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                     .setDstAccessMask(vk::AccessFlagBits2::eNone)
                                                     .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                     .setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe)
                                                     .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

            cmd.end();

            yic::timeline->finalSubmit(mSwapchain, mImageIndex, vot::SubmitInfo()
                    .setCommandBuffers(cmd)
                    .setWaitValues({vot::timelineStage::eFinish, vot::timelineStage::eFinish})
                    .setSignalValues({vot::timelineStage::ePresent, vot::timelineStage::ePresent})
                    .setWaitSemaphore(mFrameEntry->readSemaphore)
                    .setSignalSemaphore(mFrameEntry->writtenSemaphore)
                    .setWaitStageMasks({vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eColorAttachmentOutput}));

            mCurrentFrame = (mCurrentFrame + 1) % mImageCount;
        });


    }


} // rhi