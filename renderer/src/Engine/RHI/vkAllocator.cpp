//
// Created by lenovo on 6/22/2024.
//

#include "vkAllocator.h"

#include <utility>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

namespace yic {

    vkAllocator::vkAllocator() {
        auto ct = EventBus::Get::vkSetupContext();

        mDevice = ct.device_ref();
        mGraphicsQueue = ct.qGraphicsPrimary_ref();

        auto allocatorInfo = VmaAllocatorCreateInfo();
        allocatorInfo.instance = ct.instance_ref();
        allocatorInfo.physicalDevice = ct.physicalDevice_ref();
        allocatorInfo.device = ct.device_ref();

        vmaCreateAllocator(&allocatorInfo, &mVmaAllocator);

        auto transferCmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(ct.qIndexGraphicsPrimary_v())
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        mTransferPool = ct.device_ref().createCommandPool(transferCmdPoolInfo);

    }

    vkAllocator::~vkAllocator(){
        EventBus::Get::vkSetupContext().device_ref().destroy(mTransferPool);
        vmaDestroyAllocator(mVmaAllocator);
    }


    auto vkAllocator::allocBuf_impl(vk::DeviceSize deviceSize, const void *data,
                                    vk::BufferUsageFlags flags, MemoryUsage usage, const std::string& id,
                                    bool unmap) -> vkBuf_sptr {
        bufCreateInfo createInfo{
                .deviceSize = deviceSize,
                .usage = flags,

                .memoryUsage = usage,
                .allocStrategy = unmap ? AllocStrategy::eMinTime : AllocStrategy::eMapped,
        };
        auto buf = createBuf(createInfo);

        auto mapped = mapBuf(buf, deviceSize, data, unmap);

        return std::make_shared<vkBuffer>(buf.buf, buf.vmaAllocation, mapped, mVmaAllocator, id, [=](const void* src){
            memcpy(mapped, src, deviceSize);
        });
    }

    auto vkAllocator::allocBufStaging_impl(vk::DeviceSize deviceSize, const void *data,
                                            vk::BufferUsageFlags flags, const std::string& id, MemoryUsage usage, AllocStrategy allocStrategy) -> vkBuf_sptr {
        auto stagingBufCreateInfo = bufCreateInfo{
                .deviceSize = deviceSize,
                .usage = vk::BufferUsageFlagBits::eTransferSrc,
                .memoryUsage = MemoryUsage::eCpuOnly,
                .allocStrategy = AllocStrategy::eMapped,
        };
        auto stagingBuf = createBuf(stagingBufCreateInfo);
        mapBuf(stagingBuf, deviceSize, data, false);

        auto buf = createBuf({
                                     .deviceSize = deviceSize,
                                     .usage = vk::BufferUsageFlagBits::eTransferDst | flags,
                                     .memoryUsage = usage,
                                     .allocStrategy = allocStrategy,
                             });

        copyBuf(stagingBuf.buf, buf.buf, deviceSize);

        vmaUnmapMemory(mVmaAllocator, stagingBuf.vmaAllocation);
        vmaDestroyBuffer(mVmaAllocator, stagingBuf.buf, stagingBuf.vmaAllocation);

        return std::make_shared<vkBuffer>(buf.buf, buf.vmaAllocation, nullptr, mVmaAllocator, id, [this, stagingBufCreateInfo, deviceSize, buf](const void* src) {
            auto stagingBuf = createBuf(stagingBufCreateInfo);
            mapBuf(stagingBuf, deviceSize, src, false);

            copyBuf(stagingBuf.buf, buf.buf, deviceSize);

            vmaUnmapMemory(mVmaAllocator, stagingBuf.vmaAllocation);
            vmaDestroyBuffer(mVmaAllocator, stagingBuf.buf, stagingBuf.vmaAllocation);
        });
    }



    auto vkAllocator::allocImgOffScreen_impl(vkImageConfig config, const std::string& id, size_t count) -> vkImg_sptr {
        auto createImage = [&]{
            vk::ImageCreateInfo imageInfo{
                    {},
                    config.imageType,
                    config.format,
                    config.extent,
                    config.mipLevels,
                    config.arrayLayers,
                    config.sampleCountFlags,
                    config.tiling,
                    config.usage,
                    config.sharingMode
            };

            VmaAllocationCreateInfo allocInfo{
                    .usage = VMA_MEMORY_USAGE_GPU_ONLY
            };

            VkImage image;
            VmaAllocation allocation;

            vmaCreateImage(mVmaAllocator, &reinterpret_cast<const VkImageCreateInfo&>(imageInfo), &allocInfo, &image, &allocation,
                           nullptr);

            return std::make_pair(image, allocation);
        };

        auto createImageView = [&](vk::Image image) {
            vk::ImageViewCreateInfo imageViewCreateInfo{
                    {},
                    image,
                    config.imageViewType,
                    config.format,
                    config.componentSwizzle,
                    config.imageSubresourceRange
            };

            return mDevice.createImageView(imageViewCreateInfo);
        };

        auto createSampler = [&] {
            vk::SamplerCreateInfo samplerCreateInfo{
                    {},
                    config.magFilter, config.minFilter,
                    config.samplerMipMap,
                    config.u, config.v, config.w,
                    config.mipLodBias, config.anisotropyEnable, config.maxAnisotropy,
                    config.compareEnable, config.compareOp,
                    config.minLod, config.maxLod,
                    config.borderColor, config.unNormalizedCoordinates
            };

            return mDevice.createSampler(samplerCreateInfo);
        };

        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::vector<VmaAllocation> allocations;

        while (count --){
            auto [img, vma] = createImage();
            images.emplace_back(img);
            imageViews.emplace_back(createImageView(img));
            allocations.emplace_back(vma);
        }
        auto sampler = createSampler();

        return std::make_shared<vkImage>(images, imageViews, sampler, mDevice, allocations, mVmaAllocator, id);
    }

    auto vkAllocator::allocImg_impl(const imgPath& imgPath, std::optional<vkImageConfig> config, std::string id) -> vkImg_sptr {
        int w, h, c;
        vk::DeviceSize imgSize{0};
        std::vector<stbi_uc> pixels;

        std::visit([&](auto&& arg){
            using T = std::decay_t<decltype(arg)>;

            auto load = [&](const std::string & path){
                if (id.empty()){
                    size_t lastSlashPos = path.find_last_of("/\\");
                    if (lastSlashPos != std::string::npos){
                        id = path.substr(lastSlashPos + 1);
                    } else{
                        id = path;
                    }
                }

                auto data = stbi_load(path.c_str(), &w, &h, &c, STBI_rgb_alpha);
                if (data){
                    size_t size = w * h * 4;
                    pixels.insert(pixels.end(), data, data + size);
                    imgSize += size;

                    stbi_image_free(data);
                }
            };

            if constexpr (std::is_same_v<T, std::string>){
                load(arg);
            } else if constexpr (std::is_same_v<T, std::vector<std::string>>){
                for(const auto& p : arg){
                    load(p);
                }
            }
        }, imgPath);

        if (pixels.empty()){
            throw std::runtime_error("failed to load tex img");
        }

        auto stgBuf = createBuf({
            .deviceSize = imgSize,
            .usage = vk::BufferUsageFlagBits::eTransferSrc,
            .memoryUsage = MemoryUsage::eCpuOnly,
            .allocStrategy = AllocStrategy::eMapped,
        });

        mapBuf(stgBuf, imgSize, pixels.data(), false);

        auto img = allocImgOffScreen_impl(config.value().setExtent(w, h), id, 1);

        copyBufToImg(stgBuf.buf, img->images[0], (uint32_t)w, (uint32_t)h);

        vmaDestroyBuffer(mVmaAllocator, stgBuf.buf, stgBuf.vmaAllocation);

        return img;
    }

    auto vkAllocator::createBuf(const bufCreateInfo& bufCreateInfo) -> buf {
        VkBuffer buf;
        VmaAllocation allocation;

        VkBufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufCreateInfo.deviceSize,
            .usage = static_cast<VkBufferUsageFlags>(bufCreateInfo.usage),
        };

        VmaAllocationCreateInfo vmaAllocInfo{
            .flags = static_cast<VmaAllocationCreateFlags>(bufCreateInfo.allocStrategy),
            .usage = static_cast<VmaMemoryUsage>(bufCreateInfo.memoryUsage),
        };

        if (vmaCreateBuffer(mVmaAllocator, &createInfo, &vmaAllocInfo, &buf, &allocation, nullptr) != VK_SUCCESS){
            throw std::runtime_error("failed to create buf");
        }
        return {buf, allocation};
    }

    auto vkAllocator::copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize) -> void {
        createTempCmdBuf();
        vk::BufferCopy copy{0, 0, deviceSize};
        mTempCmd.copyBuffer((vk::Buffer)stagingBuf, (vk::Buffer)destBuf, 1, &copy);
        submitTempCmdBuf();
    }

    auto vkAllocator::mapBuf(const yic::vkAllocator::buf& buf, VkDeviceSize devSize, const void *data, bool unmap)-> void* {
        void* mappedData = nullptr;

        try {
            if (data == nullptr){
                mappedData = buf.vmaAllocation->GetMappedData();
            }
            if (!unmap && data != nullptr) {
                mappedData = buf.vmaAllocation->GetMappedData();
                memcpy(mappedData, data, devSize);
            } else if (unmap) {
                vmaMapMemory(mVmaAllocator, buf.vmaAllocation, &mappedData);
                memcpy(mappedData, data, devSize);
                vmaUnmapMemory(mVmaAllocator, buf.vmaAllocation);
                mappedData = nullptr;
            }
        } catch (...) {
            vmaDestroyBuffer(mVmaAllocator, buf.buf, buf.vmaAllocation);
            throw;
        }

        return mappedData;
    }

    auto vkAllocator::copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h) -> void {
        createTempCmdBuf();
        transitionImageLayout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

        vk::BufferImageCopy region{
            0, 0, 0,
            vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
            {0, 0, 0}, {w, h, 1}
        };

        mTempCmd.copyBufferToImage(buf, img, vk::ImageLayout::eTransferDstOptimal, region);

        transitionImageLayout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        submitTempCmdBuf();
    }


    auto vkAllocator::transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout,
                                            vk::ImageLayout newLayout) -> void {
        vk::ImageMemoryBarrier barrier{{}, {}, oldLayout, newLayout,
                                       vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                       vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        vk::PipelineStageFlags srcStage, dstStage;
        if (oldLayout == vk::ImageLayout::eUndefined){
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
                    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

            srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStage = vk::PipelineStageFlagBits::eTransfer;
        } else {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            srcStage = vk::PipelineStageFlagBits::eTransfer;
            dstStage = vk::PipelineStageFlagBits::eFragmentShader;
        }

        mTempCmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
    }



} // yic