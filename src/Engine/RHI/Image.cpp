//
// Created by lenovo on 10/4/2024.
//

#include "Image.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Descriptor.h"

using DefaultSampler = rhi::ImGuiDescriptorManager;

namespace vot::inline rhi {
    Image::Image(const smart_vector<vk::Image> &images, const smart_vector<vk::ImageView> &imageViews,
                 const smart_vector<VmaAllocation> &allocations, const VmaAllocator &allocator, const ImageCI &c,
                 const string &id)
        : Identifiable(id), images(images), imageViews(imageViews),
          allocations(allocations), allocator(allocator), config(c) {
        device = *yic::systemHub.va<ev::pVkSetupContext>().device;
        dispatchLoaderDynamic = *yic::systemHub.va<ev::pVkSetupContext>().dynamicDispatcher;
        if (!index)
            index = yic::systemHub.va<ev::pVkRenderContext>().activeImageIndex;
    }

    Image::Image(const smart_vector<vk::Image> &images, const smart_vector<vk::ImageView> &imageViews,
                 const smart_vector<VmaAllocation> &allocations, const vk::Image &depthImage,
                 const vk::ImageView &depthImageView, VmaAllocation const &depthAlloc, const VmaAllocator &allocator,
                 const ImageCI &c, const string &id)
        : Identifiable(id), images(images), imageViews(imageViews),
          allocations(allocations), depthImage(depthImage), depthImageView(depthImageView),
          depthAllocation(depthAlloc), allocator(allocator), config(c) {
        device = *yic::systemHub.va<ev::pVkSetupContext>().device;
        dispatchLoaderDynamic = *yic::systemHub.va<ev::pVkSetupContext>().dynamicDispatcher;
        if (!index)
            index = yic::systemHub.va<ev::pVkRenderContext>().activeImageIndex;
    }

    Image::~Image() {
        if_debug yic::logger->info("destroy {0}", id);
        for (auto &fb: framebuffers) {
            if (fb) device.destroy(fb);
        }

        if (depthImage) {
            device.destroy(depthImage);
            device.destroy(depthImageView);
            vmaFreeMemory(allocator, depthAllocation);
        }

        for (auto i = config.imageCount * config.colorAttachmentCount; i-- > 0;) {
            device.destroy(images[i]);
            device.destroy(imageViews[i]);
            vmaFreeMemory(allocator, allocations[i]);
        }
    }

    auto Image::imageInfo(const std::optional<uint32_t> imageViewIndex, const std::optional<vk::Sampler> sampler,
                          vk::ImageLayout imageLayout) const -> vk::DescriptorImageInfo {
        return {
            sampler.value_or(DefaultSampler::sampler),
            imageViews[imageViewIndex.value_or(0)],
            imageLayout
        };
    }

    auto Image::depthImageInfo(const std::optional<vk::Sampler> sampler,
                               vk::ImageLayout imageLayout) const -> vk::DescriptorImageInfo {
        return {sampler.value_or(DefaultSampler::sampler), depthImageView, imageLayout};
    }


} // rhi