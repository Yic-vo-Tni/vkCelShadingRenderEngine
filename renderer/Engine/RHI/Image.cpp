//
// Created by lenovo on 10/4/2024.
//

#include "Image.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "RHI/Descriptor.h"

using DefaultSampler = rhi::ImGuiDescriptorManager;

namespace vot::inline rhi {

    Image::Image(const smart_vector <vk::Image> &images, const smart_vector <vk::ImageView> &imageViews,
                 const smart_vector <VmaAllocation> &allocations, VmaAllocator &allocator, const vot::ImageCI& c, const string &id)
                 : images(images), imageViews(imageViews), allocations(allocations),
                 allocator(allocator), config(c), Identifiable(id){
        device = *yic::systemHub.val<ev::pVkSetupContext>().device;
        dispatchLoaderDynamic = *yic::systemHub.val<ev::pVkSetupContext>().dynamicDispatcher;
        if (!index)
            index = yic::systemHub.val<ev::pVkRenderContext>().activeImageIndex;
    }

Image::Image(const vot::smart_vector<vk::Image> &images, const vot::smart_vector<vk::ImageView> &imageViews,
             const vot::smart_vector<VmaAllocation> &allocations, const vk::Image &depthImage,
             const vk::ImageView &depthImageView, VmaAllocation const &depthAlloc, VmaAllocator &allocator,
             const vot::ImageCI &c, const vot::string &id)
        : images(images), imageViews(imageViews), allocations(allocations),
          depthImage(depthImage), depthImageView(depthImageView), depthAllocation(depthAlloc),
          allocator(allocator), config(c), Identifiable(id) {
    device = *yic::systemHub.val<ev::pVkSetupContext>().device;
    dispatchLoaderDynamic = *yic::systemHub.val<ev::pVkSetupContext>().dynamicDispatcher;
    if (!index)
        index = yic::systemHub.val<ev::pVkRenderContext>().activeImageIndex;
}

    Image::~Image() {
        std::cout << "img destroy" << id << std::endl;
        for(auto& fb : framebuffers){
            if (fb) device.destroy(fb);
        }

        if (depthImage){
            device.destroy(depthImage);
            device.destroy(depthImageView);
            vmaFreeMemory(allocator, depthAllocation);
        }

        for(auto i = config.imageCount; i-- > 0;){
            device.destroy(images[i]);
            device.destroy(imageViews[i]);
            vmaFreeMemory(allocator, allocations[i]);
        }
    }

auto Image::beginRendering(vot::CommandBuffer &cmd, vk::Rect2D rect2D) -> void {
    images.size() < *index ? activeIndex = 0 : activeIndex = *index;
    vot::vector<vk::ImageMemoryBarrier2> imageMemoryBarrier2s;
    if (config.currentImageLayout != vk::ImageLayout::eColorAttachmentOptimal && config.currentImageLayout != vk::ImageLayout::eRenderingLocalReadKHR) {
        imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(images[activeIndex])
                                                  .setOldLayout(config.currentImageLayout)
                                                  .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
    }
    if (depthImage != nullptr && config.currentDepthImageLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(depthImage)
                                                  .setOldLayout(config.currentDepthImageLayout)
                                                  .setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}));
    }
    yic::allocator->pipelineBarrier2(cmd, {}, imageMemoryBarrier2s);

    auto colorAttach = vk::RenderingAttachmentInfo()
            .setImageView(imageViews[activeIndex])
            .setImageLayout(vk::ImageLayout::eRenderingLocalReadKHR)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue{0.f, 0.f, 0.f, 0.f});

    auto renderingInfo = vk::RenderingInfo()
       //     .setFlags(vk::RenderingFlagBits::eContentsSecondaryCommandBuffers)
            .setRenderArea(rect2D)
            .setLayerCount(1)
            .setColorAttachments(colorAttach);

    if ((config.imageFlags & vot::imageFlagBits::eDepthStencil) != 0){
        auto depthStencilAttach = vk::RenderingAttachmentInfo()
                .setImageView(depthImageView)
                .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearDepthStencilValue{1.f, 0});

        renderingInfo.setPDepthAttachment(&depthStencilAttach)
            .setPStencilAttachment(&depthStencilAttach);
    }

    cmd.beginRendering(renderingInfo, dispatchLoaderDynamic);
}

auto Image::endRendering(vot::CommandBuffer &cmd) -> void {
    cmd.endRendering(dispatchLoaderDynamic);

    vot::vector<vk::ImageMemoryBarrier2> imageMemoryBarrier2s;
    if (config.currentImageLayout != vk::ImageLayout::eColorAttachmentOptimal && config.currentImageLayout != vk::ImageLayout::eRenderingLocalReadKHR) {
        imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(images[activeIndex])
                                                  .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                  .setNewLayout(config.currentImageLayout)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
    }
    if (depthImage != nullptr && config.currentDepthImageLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(depthImage)
                                                  .setOldLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                                  .setNewLayout(config.currentDepthImageLayout)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}));
    }
    yic::allocator->pipelineBarrier2(cmd, {}, imageMemoryBarrier2s);
}

auto Image::drawRendering(vot::CommandBuffer &cmd, const std::function<void()> &fn) -> void {
    beginRendering(cmd);

    fn();

    endRendering(cmd);
}



auto Image::drawRender(vot::CommandBuffer &cmd, const vot::ImageDrawCI& ci, const std::function<void()> &fn)  -> void {
    activeIndex = (*index >= images.size()) ? 0 : *index;
    yic::allocator->pipelineBarrier2(cmd, {},
                                     vk::ImageMemoryBarrier2()
                                             .setImage(images[activeIndex])
                                             .setOldLayout(ci.oldLayout)
                                             .setNewLayout(ci.newLayout)
                                             .setSrcAccessMask(ci.srcAccessMask)
                                             .setDstAccessMask(ci.dstAccessMask)
                                             .setSrcStageMask(ci.srcStageMask)
                                             .setDstStageMask(ci.dstStageMask)
                                             .setSubresourceRange(ci.subresourceRange));

    fn();

    yic::allocator->pipelineBarrier2(cmd, {},
                                     vk::ImageMemoryBarrier2()
                                             .setImage(images[activeIndex])
                                             .setOldLayout(ci.newLayout)
                                             .setNewLayout(ci.oldLayout)
                                             .setSrcAccessMask(ci.dstAccessMask)
                                             .setDstAccessMask(ci.srcAccessMask)
                                             .setSrcStageMask(ci.dstStageMask)
                                             .setDstStageMask(ci.srcStageMask)
                                             .setSubresourceRange(ci.subresourceRange));
}

auto Image::imageInfo(std::optional<uint32_t> imageViewIndex, std::optional<vk::Sampler> sampler,
                      vk::ImageLayout imageLayout) const -> vk::DescriptorImageInfo {
    return {sampler.value_or(DefaultSampler ::sampler),
            imageViews[imageViewIndex.value_or(0)],
            imageLayout};
}


} // rhi