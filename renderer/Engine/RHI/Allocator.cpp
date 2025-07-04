//
// Created by lenovo on 9/26/2024.
//

#include "Allocator.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "TimelineSemaphore.h"
#include "Command.h"
#include "Utils/FileOperation.h"
#include "Descriptor.h"

#define VMA_DEBUG_DETECT_MEMORY_LEAKS 0
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb/stb_image.h"

#include "Editor/ImGuiHub.h"

namespace rhi {


    Allocator::Allocator() {
        ct = yic::systemHub.val<ev::pVkSetupContext>();

        VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = *ct.physicalDevice,
            .device = *ct.device,
            .instance = *ct.instance,
        };

        vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator);
    }

    auto Allocator::clear() -> void {
        for(auto& [key, stgBuf] : mStagingBuffers){
            stagingBufferHandle handle;
            while(stgBuf.try_pop(handle)){
                vmaDestroyBuffer(mVmaAllocator, std::get<0>(handle), std::get<1>(handle));
                mDestroyCount ++;
            }
        }

        vmaDestroyAllocator(mVmaAllocator);
    }

    auto Allocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                                vot::memoryUsage usage, const vot::string &id, bool unmap) -> vot::Buffer_sptr {
        BufferCI ci{ .devSize = deviceSize, .flags = flags, .memoryUsage = usage, .allocStrategy = unmap ? vot::allocStrategy::eMinTime : vot::allocStrategy::eMapped};

        auto [buf, alloc] = createBuffer(ci);
        auto mapped = mapBuffer(alloc, deviceSize, data, unmap);

        return std::make_shared<vot::Buffer>(buf, alloc, mapped, mVmaAllocator, [=](const void* src){
            memcpy(mapped, src, deviceSize);
        }, id);
    }

    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                                       vot::memoryUsage usage, vot::allocStrategy strategy,
                                       const vot::string &id) -> vot::Buffer_sptr {
        BufferCI ci{ .devSize = deviceSize, .flags = vk::BufferUsageFlagBits::eTransferDst | flags, .memoryUsage = usage, .allocStrategy = strategy};
        if (data == nullptr){
            auto [buf, alloc] = createBuffer(ci);

            return std::make_shared<vot::Buffer>(buf, alloc, nullptr, mVmaAllocator, [this, deviceSize, buf](const void* src){
                auto stagingBufferHandle = acquireStagingBuffer(deviceSize);
                auto& [stgBuf, stagAlloc, s] = stagingBufferHandle;

                mapBuffer(stagAlloc, deviceSize, src, false);

                yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
                    copyBuffer(stgBuf, buf, deviceSize, cmd);
                    resetBuffer(stgBuf, cmd);
                });

                releaseStagingBuffer(stagingBufferHandle);
            }, id);
        }
        auto stagingBufferHandle = acquireStagingBuffer(deviceSize);
        auto& [stagingBuffer, stagingAlloc, size] = stagingBufferHandle;

        mapBuffer(stagingAlloc, deviceSize, data, false);

        auto [buf, alloc] = createBuffer(ci);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
            copyBuffer(stagingBuffer, buf, deviceSize, cmd);
            resetBuffer(stagingBuffer, cmd);
        });
        releaseStagingBuffer(stagingBufferHandle);

        return std::make_shared<vot::Buffer>(buf, alloc, nullptr, mVmaAllocator, [this, deviceSize, buf](const void* src){
            auto stagingBufferHandle = acquireStagingBuffer(deviceSize);
            auto& [stgBuf, stagAlloc, s] = stagingBufferHandle;

            mapBuffer(stagAlloc, deviceSize, src, false);

            yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
                copyBuffer(stgBuf, buf, deviceSize, cmd);
                resetBuffer(stgBuf, cmd);
            });

            releaseStagingBuffer(stagingBufferHandle);
        }, id);
    }

    auto Allocator::createBuffer(const Allocator::BufferCI &ci) -> bufferHandle {
        VkBuffer buf;
        VmaAllocation alloc;

        VkBufferCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = ci.devSize, .usage = static_cast<VkBufferUsageFlags>(ci.flags)};
        VmaAllocationCreateInfo allocInfo{.flags = static_cast<VmaAllocationCreateFlags>(ci.allocStrategy), .usage = static_cast<VmaMemoryUsage>(ci.memoryUsage)};
//        allocInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        if (vmaCreateBuffer(mVmaAllocator, &createInfo, &allocInfo, &buf, &alloc, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to create buf");
        return {buf, alloc};
    }

    auto Allocator::mapBuffer(VmaAllocation &alloc, VkDeviceSize devSize, const void *data,
                              bool unmap) -> void * {
        void* mapped = nullptr;

        try {
            if (data == nullptr)
                mapped = alloc->GetMappedData();
            if (!unmap && data != nullptr){
                mapped = alloc->GetMappedData();
                memcpy(mapped, data, devSize);
            } else if (unmap){
                mapped = alloc->GetMappedData();
                memcpy(mapped, data, devSize);
                vmaUnmapMemory(mVmaAllocator, alloc);
                mapped = nullptr;
            }
        } catch (...){ throw std::runtime_error("failed to mapped buf!"); }
        return mapped;
    }

    ////////////////////////////////////////////////////////////////////////////////////

    auto Allocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                                const vot::string &id) -> vot::Buffer_sptr {
        return allocBuffer(deviceSize, data, flags, vot::memoryUsage::eCpuToGpu, id);
    }

    auto Allocator::allocBuffer(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags,
                                const vot::string &id) -> vot::Buffer_sptr {
        return allocBuffer(deviceSize, nullptr, flags, id);
    }

    auto Allocator::pipelineBarrier2I(const vot::vector<vk::ImageMemoryBarrier2> &imageMemoryBarrier2,
                                     const vot::vector<vk::BufferMemoryBarrier2> &bufferMemoryBarrier2,
                                     const vot::vector<vk::MemoryBarrier2> &memoryBarrier2, vot::CommandBuffer &cmd,
                                     vk::DependencyInfo dependencyInfo) -> void {
        dependencyInfo.setImageMemoryBarriers(imageMemoryBarrier2)
                .setBufferMemoryBarriers(bufferMemoryBarrier2)
                .setMemoryBarriers(memoryBarrier2);

        cmd.pipelineBarrier2(dependencyInfo);
    }

    auto
    Allocator::acquireStagingBuffer(vk::DeviceSize deviceSize) -> stagingBufferHandle {
        auto it = mStagingBuffers.lower_bound(deviceSize);

        while(it != mStagingBuffers.end()){
            stagingBufferHandle handle;
            while(it->second.try_pop(handle)){
                return handle;
            }
            it++;
        }

        auto [buf, alloc] = createBuffer({.devSize = deviceSize, .flags = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                                          .memoryUsage = vot::memoryUsage::eCpuOnly, .allocStrategy = vot::allocStrategy::eMapped});
        mStagBufferCounter++;
        return {buf, alloc, deviceSize};
    }

    auto Allocator::releaseStagingBuffer(Allocator::stagingBufferHandle handle) -> void {
        mStagingBuffers[std::get<2>(handle)].push(handle);
    }

    auto Allocator::copyBuffer(VkBuffer stagingBuffer, VkBuffer destBuffer, VkDeviceSize deviceSize,
                               vot::CommandBuffer &cmd) -> void {
        vk::BufferCopy copy{0, 0, deviceSize};
        cmd.copyBuffer((vk::Buffer)stagingBuffer, (vk::Buffer)destBuffer, copy);
    }

    auto Allocator::resetBuffer(VkBuffer& buffer, vot::CommandBuffer &cmd) -> void {
        vk::MemoryBarrier copyBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead};
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, copyBarrier, {}, {});
        cmd.fillBuffer(buffer, 0, VK_WHOLE_SIZE, 0);
    }

    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags,
                                       const vot::string &id) -> vot::Buffer_sptr {
         return allocBufferStaging(deviceSize, data, flags, vot::memoryUsage::eGpuOnly, vot::allocStrategy::eMapped, id);
    }

    auto Allocator::loadTexture(const Allocator::imagePath &pt) -> vot::Image_sptr {
        int w, h, c;
        vk::DeviceSize imageSize{0};
        vot::vector<stbi_uc> pixels;
        vot::string id;

        std::visit([&](auto&& arg){
            using T = std::decay_t<decltype(arg)>;

            auto load = [&](const vot::string& path){
                if (id.empty()) {
                    auto last = path.find_last_of("/\\");
                    last != std::string::npos ? id = path.substr(last + 1) : id = path;
                }

                auto imageData = fo::readFile(path);
                if (imageData.empty()) throw std::runtime_error("failed to read image data");

                auto data = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &w, &h, &c, STBI_rgb_alpha);
                if (!data) throw std::runtime_error(stbi_failure_reason());

                size_t size = w * h * 4;
                pixels.insert(pixels.end(), data, data + size);
                imageSize += size;

                stbi_image_free(data);
            };

            if constexpr (std::is_same_v<T, vot::string>){
                load(arg);
            } else if constexpr (std::is_same_v<T, vot::vector<vot::string>>){
                for(const auto& p : arg){ load(p); }
            }
        }, pt);

        auto stagingBufferHandle = acquireStagingBuffer(imageSize);
        auto& [buf, alloc, devSize] = stagingBufferHandle;

        mapBuffer(alloc, imageSize, pixels.data(), false);

        auto image_sptr = allocImage(vot::ImageCI()
                .setExtent(w, h)
                .setImageCount(1), id);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
            copyBufferToImage(buf, image_sptr->images[0], w, h, cmd);
            resetBuffer(buf, cmd);
        });

        releaseStagingBuffer(stagingBufferHandle);

        return image_sptr;
    }

    auto Allocator::allocImage(vot::ImageCI config, const vot::string& id) -> vot::Image_sptr {
        auto c = config.imageCount;
        vot::smart_vector<vk::Image> images(c);
        vot::smart_vector<vk::ImageView> imageViews(c);
        vot::smart_vector<VmaAllocation> allocations(c);

        for(auto i = 0; i < c; i++){
            auto [img, alloc] = createImage(config);
            images[i] = img;
            imageViews[i] = createImageView(config, img);
            allocations[i] = alloc;
        }

        auto check = [&](const vot::imageFlags& flags) -> bool { return (config.imageFlags & flags); };
        //
        if (check(vot::imageFlagBits::eDynamicRender) && config.currentImageLayout == vk::ImageLayout::eUndefined){
            config.currentImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
        }

        if (config.currentImageLayout != vk::ImageLayout::eUndefined){
            yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
                for(auto& image : images){
                    vk::ImageMemoryBarrier barrier1{{}, vk::AccessFlagBits::eTransferWrite,
                                                   vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                                                   0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
                    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier1);
                    cmd.clearColorImage(image, vk::ImageLayout::eTransferDstOptimal, config.clearColorValue, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
                    vk::ImageMemoryBarrier barrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eColorAttachmentWrite,
                                                   vk::ImageLayout::eTransferDstOptimal, config.currentImageLayout,
                                                   0, 0, image, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
                    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {}, barrier);
                }
            });
        }

        //if (check(vot::imageFlagBits::eUpdateColorToImGui)){ yic::imguiImage->updateImage(id, imageViews); }
        if (config.uiWidget.has_value()) { yic::imguiImage->updateImage(id, imageViews); yic::imguiHub->bind(config.uiWidget.value(), [=]{ yic::imguiImage->drawImage(id); }); }

        if (check(vot::imageFlagBits::eDepthStencil)){
            auto feature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

            auto depthFormat = [&]{
                for(const auto& f : {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint}){
                    auto formatProp = ct.physicalDevice->getFormatProperties(f);
                    if ((formatProp.optimalTilingFeatures & feature) == feature)
                        return f;
                }
                return vk::Format::eD16UnormS8Uint;
            }();

            config.setFormat(depthFormat)
                    .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled)
                    .setAspect(vk::ImageAspectFlagBits::eDepth);
            auto [depthImage, depthVma] = createImage(config);
            auto depthImageView = createImageView(config, depthImage);

            return std::make_shared<vot::Image>(images, imageViews, allocations, depthImage, depthImageView, depthVma, mVmaAllocator, config, id);
        }

        return std::make_shared<vot::Image>(images, imageViews, allocations, mVmaAllocator, config, id);
    }

    auto Allocator::createImage(const vot::ImageCI &config) -> imageHandle {
        vk::ImageCreateInfo ci{
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

        VmaAllocationCreateInfo allocInfo { .usage = static_cast<VmaMemoryUsage>(vot::memoryUsage::eGpuOnly)};

        VkImage image;
        VmaAllocation alloc;

        vmaCreateImage(mVmaAllocator, &reinterpret_cast<const VkImageCreateInfo&>(ci), &allocInfo, &image, &alloc, nullptr);
        return {image, alloc};
    }

    auto Allocator::createImageView(const vot::ImageCI &config, const vk::Image &image) const -> vk::ImageView {
        vk::ImageViewCreateInfo ci{
                {},
                image,
                config.imageViewType,
                config.format,
                config.componentSwizzle,
                config.imageSubresourceRange
        };

        return ct.device->createImageView(ci);
    }

    auto Allocator::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t w, uint32_t h,
                                      vot::CommandBuffer &cmd) -> void {
        pipelineBarrier2(cmd, {}, vk::ImageMemoryBarrier2()
                .setImage(image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcAccessMask(vk::AccessFlagBits2::eNone)
                .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
                .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

        vk::BufferImageCopy copy{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, {0, 0, 0}, {w, h, 1}};
        cmd.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copy);

        pipelineBarrier2(cmd, {}, vk::ImageMemoryBarrier2()
                .setImage(image)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
    }

    auto Allocator::allocAccel(vk::AccelerationStructureCreateInfoKHR &createInfoKhr) -> vot::Accel_sptr {
        BufferCI ci{
            .devSize = createInfoKhr.size,
            .flags = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
            .memoryUsage = vot::memoryUsage::eGpuOnly,
            .allocStrategy = vot::allocStrategy::eDefault
        };

        auto [buf, alloc] = createBuffer(ci);
        createInfoKhr.buffer = buf;

        auto as = ct.device->createAccelerationStructureKHR(createInfoKhr, nullptr, *ct.dynamicDispatcher);

        return std::make_shared<vot::Accel>(buf, alloc, mVmaAllocator, as, IdGenerator::uniqueId());
    }

    auto Allocator::allocAccel(vk::AccelerationStructureBuildSizesInfoKHR buildSizesInfoKhr,
                               vk::AccelerationStructureTypeKHR type) -> vot::Accel_sptr {
        return allocAccel(vk::AccelerationStructureCreateInfoKHR()
                                  .setType(type)
                                  .setSize(buildSizesInfoKhr.accelerationStructureSize));
    }

    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags,
                                       const vot::string &id) -> vot::Buffer_sptr {
        return allocBufferStaging(deviceSize, nullptr, flags, vot::memoryUsage::eGpuOnly, vot::allocStrategy::eMapped, id);
    }

    auto Allocator::allocDedicatedBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags,
                                                const vot::string &id) -> vot::Buffer_sptr {
        return allocBufferStaging(deviceSize, nullptr, flags, vot::memoryUsage::eGpuOnly,
                                  static_cast<vot::allocStrategy>(vot::allocStrategy::eMapped |
                                                                  vot::allocStrategy::eDedicated), id);
    }


} // rhi