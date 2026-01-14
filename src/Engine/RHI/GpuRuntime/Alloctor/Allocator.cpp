//
// Created by lenovo on 9/26/2024.
//

#include "Allocator.h"
#include "Prototype/Allocator2.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/TimelineSemaphore.h"
#include "RHI/Command.h"
#include "Utils/FileOperation.h"
#include "RHI/Descriptor.h"

#define VMA_IMPLEMENTATION
#define VMA_DEBUG_DETECT_LEAKS 1
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_ALLOCATOR_CREATE_DEBUG_MARGIN_BIT 0x00000020
#define VMA_ALLOCATOR_CREATE_DEBUG_DETECT_CORRUPTION_BIT 0x00000040
#define VMA_ALLOCATOR_CREATE_DEBUG_ALLOCATIONS_BIT 0x00000080
#include "vma/vk_mem_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "stb/stb_image.h"

#include "Editor/ImGuiHub.h"

namespace rhi {
    Allocator::Allocator() {
        //clang-format off
        ct = yic::systemHub.va<ev::pVkSetupContext>();

        {  const VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT
                         | VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT
                         | VMA_ALLOCATOR_CREATE_DEBUG_MARGIN_BIT
                         | VMA_ALLOCATOR_CREATE_DEBUG_DETECT_CORRUPTION_BIT
                         | VMA_ALLOCATOR_CREATE_DEBUG_ALLOCATIONS_BIT,
                .physicalDevice = *ct.physicalDevice,
                .device = *ct.device,
                .instance = *ct.instance,
            };
            vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator);
        } //

        mCaches = std::make_unique<vot::gfx::LRUStagingBufferCache>(mVmaAllocator);

        {   yic::systemHub.sub([&](ev::tDestroyVMA) {
               VmaTotalStatistics totalStats{};
               vmaCalculateStatistics(mVmaAllocator, &totalStats);

               if_debug std::cout
                       << "Total allocations: " << totalStats.total.statistics.allocationCount
                       << ", total bytes: " << totalStats.total.statistics.allocationBytes
                       << std::endl;
            });
        } //
        //clang-format on
    }

    auto Allocator::clear() -> void {
        yic::systemHub.pub(ev::tDestroyVMA{});
        mCaches.reset();
        yic::systemHub.pub(ev::tDestroyVMA{});
        vmaDestroyAllocator(mVmaAllocator);
    }

    auto Allocator::buildBuffer(const vot::gfx::api::BufferCI &ci) -> vot::Buffer_sptr {
        vot::dsl::Match{ci.residency}
                .case_(vot::gfx::api::BufferResidency::eDefault, [&] {
                    return allocBuffer(ci.device_size, ci.data, ci.buffer_usage_flags);
                })
                .case_(vot::gfx::api::BufferResidency::eStaging, [&] {
                    return allocBufferStaging(ci.device_size, ci.data, ci.buffer_usage_flags);
                })
                .case_(vot::gfx::api::BufferResidency::eDedicated, [&] {
                    return allocDedicatedBufferStaging(ci.device_size, ci.buffer_usage_flags);
                });
        // TODO:
        return nullptr;
    }

    auto Allocator::allocBuffer(const vk::DeviceSize deviceSize, const void *data, const vk::BufferUsageFlags flags,
                                const vot::memoryUsage usage, const vot::string &id, const bool unmap) -> vot::Buffer_sptr {
        const BufferCI ci{ .devSize = deviceSize, .flags = flags, .memoryUsage = usage, .allocStrategy = unmap ? vot::allocStrategy::eMinTime : vot::allocStrategy::eMapped};
        // NOTE: unmap parm is not support now
        auto [buf, alloc] = createBuffer(ci);
        auto mapped = mapBuffer(alloc, deviceSize, data);

        return std::make_shared<vot::Buffer>(buf, alloc, mapped, mVmaAllocator, [=](const void* src){
            memcpy(mapped, src, deviceSize);
        }, id);
    }

    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, const void *data, const vk::BufferUsageFlags flags,
                                       const vot::memoryUsage usage, const vot::allocStrategy strategy,
                                       const vot::string &id) -> vot::Buffer_sptr {
        const BufferCI ci{
            .devSize      = deviceSize,
            .flags        = vk::BufferUsageFlagBits::eTransferDst | flags,
            .memoryUsage  = usage,
            .allocStrategy = strategy
        };
        auto [buf, alloc] = createBuffer(ci);

        auto uploadViaStaging = [this, deviceSize, buf](const void* src) {
            auto stagingHandle = mCaches->acquire(deviceSize);
            auto& [stagingBuffer, stagingAlloc, size] = stagingHandle;

            mapBuffer(stagingAlloc, deviceSize, src);

            yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
                copyBuffer(stagingBuffer, buf, deviceSize, cmd);
                resetBuffer(stagingBuffer, cmd);
            });
        };

        if (data != nullptr) uploadViaStaging(data);

        return std::make_shared<vot::Buffer>(buf, alloc, nullptr, mVmaAllocator, uploadViaStaging, id);
    }

    auto Allocator::createBuffer(const Allocator::BufferCI &ci) const -> vot::gfx::detail::BufferAllocation {
        VkBuffer buf;
        VmaAllocation alloc;

        const VkBufferCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = ci.devSize, .usage = static_cast<VkBufferUsageFlags>(ci.flags)};
        const VmaAllocationCreateInfo allocInfo{.flags = static_cast<VmaAllocationCreateFlags>(ci.allocStrategy), .usage = static_cast<VmaMemoryUsage>(ci.memoryUsage)};

        if (vmaCreateBuffer(mVmaAllocator, &createInfo, &allocInfo, &buf, &alloc, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to create buf");
        return {buf, alloc};
    }

    auto Allocator::mapBuffer(const VmaAllocation &alloc, const VkDeviceSize devSize, const void *data) -> void * {
        void* mapped = nullptr;

        try {
            if (data == nullptr)
                mapped = alloc->GetMappedData();
            if (data != nullptr){
                mapped = alloc->GetMappedData();
                memcpy(mapped, data, devSize);
            }
        } catch (...){ throw std::runtime_error("failed to mapped buf!"); }
        return mapped;
    }

    auto Allocator::allocBuffer(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, const vot::string &id) -> vot::Buffer_sptr {
        return allocBuffer(deviceSize, data, flags, vot::memoryUsage::eCpuToGpu, id);
    }
    auto Allocator::allocBuffer(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string &id) -> vot::Buffer_sptr {
        return allocBuffer(deviceSize, nullptr, flags, id);
    }
    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, const vot::string &id) -> vot::Buffer_sptr {
        return allocBufferStaging(deviceSize, data, flags, vot::memoryUsage::eGpuOnly, vot::allocStrategy::eDefault, id);
    }
    auto Allocator::allocBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string &id) -> vot::Buffer_sptr {
        return allocBufferStaging(deviceSize, nullptr, flags, vot::memoryUsage::eGpuOnly, vot::allocStrategy::eDefault, id);
    }
    auto Allocator::allocDedicatedBufferStaging(vk::DeviceSize deviceSize, vk::BufferUsageFlags flags, const vot::string &id) -> vot::Buffer_sptr {
        return allocBufferStaging(deviceSize, nullptr, flags, vot::memoryUsage::eGpuOnly, static_cast<vot::allocStrategy>(vot::allocStrategy::eDedicated), id);
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

    auto Allocator::loadTexture(const Allocator::imagePath &pt) -> vot::Image_sptr {
        int w, h, c;
        vk::DeviceSize imageSize{0};
        vot::vector<stbi_uc> pixels;
        vot::string id;

        std::visit([&]<typename T0>(T0&& arg){
            using T = std::decay_t<T0>;

            auto load = [&](const vot::string& path){
                if (id.empty()) {
                    auto last = path.find_last_of("/\\");
                    last != std::string::npos ? id = path.substr(last + 1) : id = path;
                }

                const auto imageData = fo::readFile(path);
                if (imageData.empty()) throw std::runtime_error("failed to read image data");

                const auto data = stbi_load_from_memory(imageData.data(), static_cast<int>(imageData.size()), &w, &h, &c, STBI_rgb_alpha);
                if (!data) throw std::runtime_error(stbi_failure_reason());

                const size_t size = w * h * 4;
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

        auto stagingBufferHandle = mCaches->acquire(imageSize);
        auto& [buf, alloc, devSize] = stagingBufferHandle;

        mapBuffer(alloc, imageSize, pixels.data());

        auto image_sptr = allocImage(vot::ImageCI()
                .setExtent(w, h)
                .setImageCount(1), id);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
            copyBufferToImage(buf, image_sptr->images[0], w, h, cmd);
            resetBuffer(buf, cmd);
        });

        return image_sptr;
    }

    auto Allocator::allocImage(vot::ImageCI config, const vot::string& id) -> vot::Image_sptr {
        //clang-format off
        auto c = config.imageCount * config.colorAttachmentCount;
        vot::smart_vector<vk::Image> images(c);
        vot::smart_vector<vk::ImageView> imageViews(c);
        vot::smart_vector<VmaAllocation> allocations(c);
        auto check = [&](const vot::imageFlags& flags) -> bool { return (config.imageFlags & flags); };

        {   for (auto i = 0; i < c; i++) {
                auto [img, alloc] = createImage(config);
                images[i] = img;
                imageViews[i] = createImageView(config, img);
                allocations[i] = alloc;
            }
        } // create color images

        {   if (check(vot::imageFlagBits::eDynamicRender) && config.currentImageLayout == vk::ImageLayout::eUndefined){
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
        } // fix layout

        {   if (config.uiWidget.has_value()) {
                yic::imguiImage->updateImage(id, imageViews);
                yic::imguiHub->bind(config.uiWidget.value(), [=] {
                    yic::imguiImage->drawImage(id);

                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        const ImVec2 min = ImGui::GetItemRectMin();
                        const ImVec2 max = ImGui::GetItemRectMax();
                        const ImVec2 mouse = ImGui::GetMousePos();

                        float u = (mouse.x - min.x) / (max.x - min.x);
                        float v = (mouse.y - min.y) / (max.y - min.y);

                        u = std::clamp(u, 0.0f, 1.0f);
                        v = std::clamp(v, 0.0f, 1.0f);

                        GLOBAL::mousePick = std::pair(u, v);

                        yic::logger->warn("Mouse pick u:{0}, v{1}", u, v);
                    }
                });
            }
        }   // optional: UI bind /// NOTE: just only support [one window <-> one image] display

        {   if (check(vot::imageFlagBits::eDepthStencil)) {
                auto feature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

                auto depthFormat = [&] {
                    for (const auto &f: { vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint}) {
                        auto formatProp = ct.physicalDevice->getFormatProperties(f);
                        if ((formatProp.optimalTilingFeatures & feature) == feature) return f;}
                    return vk::Format::eD16UnormS8Uint;
                }(); // choose depth format

                config.setFormat(depthFormat)
                        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled |
                                  vk::ImageUsageFlagBits::eInputAttachment | vk::ImageUsageFlagBits::eTransferDst)
                        .setAspect(vk::ImageAspectFlagBits::eDepth);
                auto [depthImage, depthVma] = createImage(config);
                auto depthImageView = createImageView(config, depthImage);

                if (config.currentDepthImageLayout != vk::ImageLayout::eDepthStencilAttachmentOptimal) {
                    yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
                        const vk::ImageMemoryBarrier barrier1{
                            {}, vk::AccessFlagBits::eTransferWrite,
                            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
                            0, 0, depthImage,
                            {vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}
                        };
                        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {},
                                            {}, {}, barrier1);
                        const vk::ClearDepthStencilValue clearValue{1.0f, 0};
                        const vk::ImageSubresourceRange subRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1};
                        cmd.clearDepthStencilImage(depthImage, vk::ImageLayout::eTransferDstOptimal, clearValue, subRange);

                        const vk::ImageMemoryBarrier barrier{
                            vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eColorAttachmentWrite,
                            vk::ImageLayout::eTransferDstOptimal, config.currentDepthImageLayout,
                            0, 0, depthImage,
                            {vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}
                        };
                        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                            vk::PipelineStageFlagBits::eColorAttachmentOutput, {}, {}, {}, barrier);
                    });
                }

                return std::make_shared<vot::Image>(images, imageViews, allocations, depthImage, depthImageView, depthVma,mVmaAllocator, config, id);
            }
        } // optional: depth image

        //clang-format on
        return std::make_shared<vot::Image>(images, imageViews, allocations, mVmaAllocator, config, id);
    }

    auto Allocator::uploadImage(const vk::Image& image, void* data, vk::Extent3D extent, vk::Format format) -> void {
        size_t pixelSize = (format == vk::Format::eR16Sfloat ? 2 : 4);
        size_t totalBytes = extent.width * extent.height * extent.depth * pixelSize;
        auto stagingBufferHandle = mCaches->acquire(totalBytes);
        auto& [stagingBuffer, stagingAlloc, devSize] = stagingBufferHandle;

        mapBuffer(stagingAlloc, totalBytes, data);

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
            pipelineBarrier2(cmd, {}, vk::ImageMemoryBarrier2()
                    .setImage(image)
                    .setOldLayout(vk::ImageLayout::eUndefined)
                    .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setSrcAccessMask(vk::AccessFlagBits2::eNone)
                    .setDstAccessMask(vk::AccessFlagBits2::eTransferWrite)
                    .setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe)
                    .setDstStageMask(vk::PipelineStageFlagBits2::eTransfer)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));

            vk::BufferImageCopy copy{0, 0, 0,
                                     vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                                     {0, 0, 0},
                                     extent
            };
            cmd.copyBufferToImage(stagingBuffer, image, vk::ImageLayout::eTransferDstOptimal, copy);

            pipelineBarrier2(cmd, {}, vk::ImageMemoryBarrier2()
                    .setImage(image)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                    .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite)
                    .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)
                    .setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer)
                    .setDstStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                    .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}));
        });
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





} // rhi2


// const BufferCI ci{ .devSize = deviceSize, .flags = vk::BufferUsageFlagBits::eTransferDst | flags, .memoryUsage = usage, .allocStrategy = strategy};
// if (data == nullptr){
//     auto [buf, alloc] = createBuffer(ci);
//
//     return std::make_shared<vot::Buffer>(buf, alloc, nullptr, mVmaAllocator, [this, deviceSize, buf](const void* src){
//         auto stagingBufferHandle = mCaches->acquire(deviceSize);
//         auto& [stgBuf, stagAlloc, s] = stagingBufferHandle;
//
//         mapBuffer(stagAlloc, deviceSize, src);
//
//         yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
//             copyBuffer(stgBuf, buf, deviceSize, cmd);
//             resetBuffer(stgBuf, cmd);
//         });
//
//     }, id);
// }
// auto stagingBufferHandle = mCaches->acquire(deviceSize);
// auto& [stagingBuffer, stagingAlloc, size] = stagingBufferHandle;
//
// mapBuffer(stagingAlloc, deviceSize, data);
//
// auto [buf, alloc] = createBuffer(ci);
//
// yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
//     copyBuffer(stagingBuffer, buf, deviceSize, cmd);
//     resetBuffer(stagingBuffer, cmd);
// });
//
// return std::make_shared<vot::Buffer>(buf, alloc, nullptr, mVmaAllocator, [this, deviceSize, buf](const void* src){
//     auto stagingBufferHandle = mCaches->acquire(deviceSize);
//     auto& [stgBuf, stagAlloc, s] = stagingBufferHandle;
//
//     mapBuffer(stagAlloc, deviceSize, src);
//
//     yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd){
//         copyBuffer(stgBuf, buf, deviceSize, cmd);
//         resetBuffer(stgBuf, cmd);
//     });
//
// }, id);