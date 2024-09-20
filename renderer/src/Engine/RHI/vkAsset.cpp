//
// Created by lenovo on 9/19/2024.
//

#include "vkAsset.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic2{

    auto Image::init() -> void {
        mDevice = *mg::SystemHub.val<ev::pVkSetupContext>().device;
        mDynamicDispatcher = *mg::SystemHub.val<ev::pVkSetupContext>().dynamicDispatcher;
        mIndex = mg::SystemHub.val<ev::hVkRenderContext>(toolkit::enum_name(RenderPhase::ePrimary)).activeImageIndex;
    }

    Image::Image(const vot::smart_vector<vk::Image> &images, const vot::smart_vector<vk::ImageView> &imageViews,
                 const vot::smart_vector<VmaAllocation> &vmaAlloc, VmaAllocator &vmaAllocator, const vk::Device &device,
                 ImageConfig config, const std::string &id)
            : images(images), imageViews(imageViews), vmaAllocations(vmaAlloc), vmaAllocator(vmaAllocator),
              mDevice(device), config(config), Identifiable(id) {
        init();
        counter++;
    }

    Image::Image(const vot::smart_vector<vk::Image> &images, const vot::smart_vector<vk::ImageView> &imageViews,
                 const vot::smart_vector<VmaAllocation> &vmaAlloc, VmaAllocator &vmaAllocator,
                 const vk::Image &depthImage, const vk::ImageView &depthImageView, VmaAllocation const &depthVmaAlloc,
                 const vk::Device &device, ImageConfig config, const std::string &id)
            : images(images), imageViews(imageViews), vmaAllocations(vmaAlloc), vmaAllocator(vmaAllocator),
              depthImage(depthImage), depthImageView(depthImageView), depthVmaAllocation(depthVmaAlloc),
              mDevice(device), config(config), Identifiable(id) {
        init();
        counter++;
    }

    Image::Image(const vot::smart_vector<vk::Image> &images, const vot::smart_vector<vk::ImageView> &imageViews,
                 const vot::smart_vector<VmaAllocation> &vmaAlloc, VmaAllocator &vmaAllocator,
                 const vk::Image &depthImage, const vk::ImageView &depthImageView, VmaAllocation const &depthVmaAlloc,
                 const vot::smart_vector<vk::Framebuffer> &framebuffers, const vk::Device &device, ImageConfig config,
                 const std::string &id)
            : images(images), imageViews(imageViews), vmaAllocations(vmaAlloc), vmaAllocator(vmaAllocator),
              depthImage(depthImage), depthImageView(depthImageView), depthVmaAllocation(depthVmaAlloc),
              framebuffers(framebuffers), mDevice(device), config(config), Identifiable(id) {
        init();
        counter++;
    };

    auto Image::beginRendering(vk::CommandBuffer& cmd, vk::Rect2D rect2D) -> void {
        auto colorAttach = vk::RenderingAttachmentInfo()
                .setImageView(imageViews[*mIndex])
                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearColorValue{0.f, 0.f, 0.f, 0.f});

        auto renderingInfo = vk::RenderingInfo()
                .setRenderArea(rect2D)
                .setLayerCount(1)
                .setColorAttachments(colorAttach);

        if ((config.imageFlags & ImageFlagBits::eDepthStencil) != 0) {
            auto depthStencilAttach = vk::RenderingAttachmentInfo()
                    .setImageView(depthImageView)
                    .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                    .setClearValue(vk::ClearDepthStencilValue{1.f, 0});

            renderingInfo.setPDepthAttachment(&depthStencilAttach)
                    .setPStencilAttachment(&depthStencilAttach);
        }

        cmd.beginRendering(renderingInfo, mDynamicDispatcher);
    }

    auto Image::dynamicRendering(vk::CommandBuffer &cmd, const std::function<void()>& fn) -> void {
        beginRendering(cmd);
        fn();
        endRendering(cmd);
    }

    auto Image::dynamicRendering(vk::CommandBuffer &cmd, vk::Rect2D rect2D, const std::function<void()> &fn) -> void {
        beginRendering(cmd, rect2D);
        fn();
        endRendering(cmd);
    }

    auto Image::endRendering(vk::CommandBuffer &cmd) -> void {
        cmd.endRendering(mDynamicDispatcher);
    }

}
