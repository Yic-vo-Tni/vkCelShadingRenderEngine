//
// Created by lenovo on 11/9/2025.
//

#include "RenderTarget.h"

#include "RHI/GpuRuntime/Alloctor/Allocator.h"
#include "RHI/Image.h"


namespace runtime::flow {

    auto RenderTarget::draw(vot::CommandBuffer &cmd, const std::function<void()> &fn) -> void {
        if (RT->config.srcImageLayout != vk::ImageLayout::eUndefined) {
            drawRender(cmd, fn);
        } else {
            drawRendering(cmd, fn);
        }
    }

    auto RenderTarget::beginRendering(vot::CommandBuffer &cmd)  -> void {
        RT->images.size() / RT->config.colorAttachmentCount < *vot::Image::index ? activeIndex = 0 : activeIndex = *vot::Image::index;
        vot::vector<vk::ImageMemoryBarrier2> imageMemoryBarriers;
        if (RT->config.currentImageLayout != vk::ImageLayout::eColorAttachmentOptimal && RT->config.currentImageLayout != vk::ImageLayout::eRenderingLocalRead) {
            for (auto i = 0; i < RT->config.colorAttachmentCount; i++) {
                imageMemoryBarriers.emplace_back(vk::ImageMemoryBarrier2()
                                                    .setImage(RT->images[activeIndex + i])
                                                    .setOldLayout(RT->config.currentImageLayout)
                                                    .setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                    .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                    .setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                    .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                    .setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                    .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

            }
        }
        if (RT->depthImage != nullptr && RT->config.currentDepthImageLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            imageMemoryBarriers.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(RT->depthImage)
                                                  .setOldLayout(RT->config.currentDepthImageLayout)
                                                  .setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}));
        }
        yic::allocator->pipelineBarrier2(cmd, {}, imageMemoryBarriers);

        vot::vector<vk::RenderingAttachmentInfo> colorAttachments(RT->config.colorAttachmentCount);
        const auto aI = activeIndex * RT->config.colorAttachmentCount;
        for(auto i = 0; i < RT->config.colorAttachmentCount; i++){
            colorAttachments[i] = vk::RenderingAttachmentInfo()
                .setImageView(RT->imageViews[aI + i])
                .setImageLayout(vk::ImageLayout::eRenderingLocalReadKHR)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearColorValue{0.f, 0.f, 0.f, 0.f});
        }

        auto renderingInfo = vk::RenderingInfo()
            .setRenderArea({RT->config.renderAreaOffset, {RT->config.extent.width, RT->config.extent.height}})
            .setLayerCount(1)
            .setColorAttachments(colorAttachments);

        vk::RenderingAttachmentInfo depthStencilAttach;
        if ((RT->config.imageFlags & vot::imageFlagBits::eDepthStencil) != 0){
            depthStencilAttach
                .setImageView(RT->depthImageView)
                .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setLoadOp(vk::AttachmentLoadOp::eClear)
                .setStoreOp(vk::AttachmentStoreOp::eStore)
                .setClearValue(vk::ClearDepthStencilValue{1.f, 0});

            renderingInfo
                .setPDepthAttachment(&depthStencilAttach)
                .setPStencilAttachment(nullptr);
        }

        cmd.beginRendering(renderingInfo, RT->dispatchLoaderDynamic);
    }

    auto RenderTarget::endRendering(vot::CommandBuffer &cmd)  -> void {
        cmd.endRendering(RT->dispatchLoaderDynamic);

        vot::vector<vk::ImageMemoryBarrier2> imageMemoryBarrier2s;
        if (RT->config.currentImageLayout != vk::ImageLayout::eColorAttachmentOptimal && RT->config.currentImageLayout != vk::ImageLayout::eRenderingLocalReadKHR) {
            for(auto i = 0; i < RT->config.colorAttachmentCount; i++){
                imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                      .setImage(RT->images[activeIndex + i])
                                                      .setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)
                                                      .setNewLayout(RT->config.currentImageLayout)
                                                      .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite)
                                                      .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                      .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                                                      .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                      .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
            }
        }
        if (RT->depthImage != nullptr && RT->config.currentDepthImageLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            imageMemoryBarrier2s.emplace_back(vk::ImageMemoryBarrier2()
                                                  .setImage(RT->depthImage)
                                                  .setOldLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                                                  .setNewLayout(RT->config.currentDepthImageLayout)
                                                  .setSrcAccessMask(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                                                  .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests)
                                                  .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                                                  .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}));
        }
        yic::allocator->pipelineBarrier2(cmd, {}, imageMemoryBarrier2s);
    }

    auto RenderTarget::drawRendering(vot::CommandBuffer &cmd, const std::function<void()> &fn)  -> void {
        beginRendering(cmd);

        fn();

        endRendering(cmd);
    }


    auto RenderTarget::drawRender(vot::CommandBuffer &cmd, const std::function<void()> &fn)  -> void {
        activeIndex = (*vot::Image::index >= (RT->images.size() / RT->config.colorAttachmentCount)) ? 0 : *vot::Image::index;
        //FIXME 未添加多color attachment ↓，如需要 需修改
        //NOTE 如果未来有不需要更改布局的情况，则需要优化此处
        yic::allocator->pipelineBarrier2(cmd, {},
                                         vk::ImageMemoryBarrier2()
                                         .setImage(RT->images[activeIndex])
                                         .setOldLayout(RT->config.currentImageLayout)
                                         .setNewLayout(RT->config.srcImageLayout)
                                         .setSrcAccessMask(RT->config.srcAccessMask)
                                         .setDstAccessMask(RT->config.dstAccessMask)
                                         .setSrcStageMask(RT->config.srcStageMask)
                                         .setDstStageMask(RT->config.dstStageMask)
                                         .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

        fn();

        yic::allocator->pipelineBarrier2(cmd, {},
                                         vk::ImageMemoryBarrier2()
                                         .setImage(RT->images[activeIndex])
                                         .setOldLayout(RT->config.srcImageLayout)
                                         .setNewLayout(RT->config.currentImageLayout)
                                         .setSrcAccessMask(RT->config.dstAccessMask)
                                         .setDstAccessMask(RT->config.srcAccessMask)
                                         .setSrcStageMask(RT->config.dstStageMask)
                                         .setDstStageMask(RT->config.srcStageMask)
                                         .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
    }
} // runtime