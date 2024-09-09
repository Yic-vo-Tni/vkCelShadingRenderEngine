//
// Created by lenovo on 6/22/2024.
//

#include "Allocator.h"

#include <utility>

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

namespace yic {
    uint32_t Allocator::mMainRenderImageCount = 0;

    Allocator::Allocator() {
        ct = mg::SystemHub.val<ev::pVkSetupContext>();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);

        mDevice = *ct.device;
        mDyDispatcher = *ct.dynamicDispatcher;
        mMainRenderImageCount = rt.imageCount_v();

        FixSampler::get();

        auto allocatorInfo = VmaAllocatorCreateInfo();
        //allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocatorInfo.instance = *ct.instance;
        allocatorInfo.physicalDevice = *ct.physicalDevice;
        allocatorInfo.device = *ct.device;

        vmaCreateAllocator(&allocatorInfo, &mVmaAllocator);

        CommandBufferCoordinator::init(mDevice, ct.queueFamily->gIndexAuxiliary(), ct.queueFamily->gAuxiliary());
    }

    Allocator::~Allocator(){
        vmaDestroyAllocator(mVmaAllocator);
    }


    auto Allocator::allocBuf_impl(vk::DeviceSize deviceSize, const void *data,
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

    auto Allocator::allocBufStaging_impl(vk::DeviceSize deviceSize, const void *data,
                                            vk::BufferUsageFlags flags, const std::string& id, MemoryUsage usage, AllocStrategy allocStrategy) -> vkBuf_sptr {

        if (data == nullptr){
            auto buf = createBuf({
                                         .deviceSize = deviceSize,
                                         .usage = vk::BufferUsageFlagBits::eTransferDst | flags,
                                         .memoryUsage = usage,
                                         .allocStrategy = allocStrategy,
            });

            return std::make_shared<vkBuffer>(buf.buf, buf.vmaAllocation, nullptr, mVmaAllocator, id, [this, deviceSize, buf](const void* src){
                auto stagingBuf = acquireStagingBuf(deviceSize);
                mapBuf(stagingBuf.stgBuf, deviceSize, src, false);

                CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
                    copyBuf(stagingBuf.stgBuf.buf, buf.buf, deviceSize, cmd);
                    resetBuf(stagingBuf, cmd);
                });

                releaseStagingBuf(stagingBuf);
            });
        }
        auto stagingBuf = acquireStagingBuf(deviceSize);

        mapBuf(stagingBuf.stgBuf, deviceSize, data, false);

        auto buf = createBuf({
                                     .deviceSize = deviceSize,
                                     .usage = vk::BufferUsageFlagBits::eTransferDst | flags,
                                     .memoryUsage = usage,
                                     .allocStrategy = allocStrategy,
                             });

        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
            copyBuf(stagingBuf.stgBuf.buf, buf.buf, deviceSize, cmd);
            resetBuf(stagingBuf, cmd);
        });
        releaseStagingBuf(stagingBuf);

        return std::make_shared<vkBuffer>(buf.buf, buf.vmaAllocation, nullptr, mVmaAllocator, id, [this, deviceSize, buf](const void* src) {
            auto stagingBuf = acquireStagingBuf(deviceSize);
            mapBuf(stagingBuf.stgBuf, deviceSize, src, false);

            CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
                copyBuf(stagingBuf.stgBuf.buf, buf.buf, deviceSize, cmd);
                resetBuf(stagingBuf, cmd);
            });

            releaseStagingBuf(stagingBuf);
        });

    }



    auto Allocator::allocImgOffScreen_impl(vkImageConfig config, const std::string& id, size_t count) -> vkImg_sptr {
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

        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::vector<VmaAllocation> allocations;
        images.reserve(count);
        imageViews.reserve(count);
        allocations.reserve(count);

        while (count --){
            auto [img, vma] = createImage();
            images.emplace_back(img);
            imageViews.emplace_back(createImageView(img));
            allocations.emplace_back(vma);
        }

        if (config.imageFlags == vkImageConfig::eColor){
            auto img = std::make_shared<vkImage>(images, imageViews, mDevice, allocations, mVmaAllocator, config,id);
//            EventBus::update(et::vkResource{
//                .img = img
//            });
            return img;
        }
        if (config.imageFlags == vkImageConfig::eStorage){
            vk::ImageMemoryBarrier barrier{
                    {}, vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
                    0, 0, images[0], {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
            };
            CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
               cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR, {},
                                   {}, {}, barrier);
            });

            auto img = std::make_shared<vkImage>(images, imageViews, mDevice, allocations, mVmaAllocator, config, id);
            return img;
        }
        if ((config.imageFlags & (vkImageConfig::eDepthStencil | vkImageConfig::eColor)) != 0){
            auto feature = vk::FormatFeatureFlagBits::eDepthStencilAttachment;

            auto depthFormat = [&] {
                for (const auto &f: {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint,
                                     vk::Format::eD16UnormS8Uint}) {
                    auto formatProp = ct.physicalDevice->getFormatProperties(f);
                    if ((formatProp.optimalTilingFeatures & feature) == feature) {
                        return f;
                    }
                }
                return vk::Format::eD16UnormS8Uint;
            }();

            config.setFormat(depthFormat).setUsage(
                    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled).setAspect(
                    vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil);
            auto [depthImage, depthVma] = createImage();
            auto depthImageView = createImageView(depthImage);


            if (config.renderPass) {
                std::vector<vk::Framebuffer> framebuffers;
                for (size_t i = 0; i < images.size(); i++) {
                    std::vector<vk::ImageView> views;
                    views.emplace_back(imageViews[i]);
                    views.emplace_back(depthImageView);

                    auto createInfo = vk::FramebufferCreateInfo().setRenderPass(config.renderPass)
                            .setWidth(config.extent.width)
                            .setHeight(config.extent.height)
                            .setLayers(1)
                            .setAttachments(views);

                    framebuffers.emplace_back(mDevice.createFramebuffer(createInfo));
                }

                auto img = std::make_shared<vkImage>(images, imageViews, allocations,
                                                 depthImage, depthImageView, depthVma,
                                                 framebuffers,mDevice, mVmaAllocator, config, id);
                //EventBus::update(et::vkResource{.img = img});
                return img;
            }
            auto img = std::make_shared<vkImage>(images, imageViews, allocations, depthImage, depthImageView, depthVma,
                                             mDevice, mVmaAllocator, config, id);
            //EventBus::update(et::vkResource{.img = img});
            return img;
        }

        throw std::runtime_error("failed to create off img");
    }

    auto Allocator::allocImg_impl(const imgPath& imgPath, std::optional<vkImageConfig> config, std::string id) -> vkImg_sptr {
        int w, h, c;
        vk::DeviceSize imgSize{0};
        std::vector<stbi_uc> pixels;

        auto readFile = [&](const std::wstring& path) -> std::vector<unsigned char> {
            HANDLE fileHandle = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (fileHandle == INVALID_HANDLE_VALUE) {
                std::cerr << "Failed to open file" << std::endl;
                return {};
            }

            LARGE_INTEGER fileSize;
            if (!GetFileSizeEx(fileHandle, &fileSize)) {
                CloseHandle(fileHandle);
                std::cerr << "Failed to get file size" << std::endl;
                return {};
            }

            std::vector<unsigned char> buffer(static_cast<size_t>(fileSize.QuadPart));
            DWORD bytesRead;
            if (!ReadFile(fileHandle, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, nullptr) || bytesRead != fileSize.QuadPart) {
                CloseHandle(fileHandle);
                std::cerr << "Failed to read file" << std::endl;
                return {};
            }

            CloseHandle(fileHandle);
            return buffer;
        };

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

                auto pt = boost::locale::conv::utf_to_utf<wchar_t>(path);
                auto imgData = readFile(pt);
                if (imgData.empty())
                    vkError("failed to read img data");
                auto data = stbi_load_from_memory(imgData.data(), static_cast<int>(imgData.size()), &w, &h, &c, STBI_rgb_alpha);
                if (data == nullptr){
                    vkError(stbi_failure_reason());
                }
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

        auto stgBuf = acquireStagingBuf(imgSize);
        mapBuf(stgBuf.stgBuf, imgSize, pixels.data(), false);

        auto img = allocImgOffScreen_impl(config.value().setExtent(w, h), id, 1);

        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
            copyBufToImg(stgBuf.stgBuf.buf, img->images[0], (uint32_t)w, (uint32_t)h, cmd);
            resetBuf(stgBuf, cmd);
        });

        releaseStagingBuf(stgBuf);

        return img;
    }

    auto Allocator::createBuf(const bufCreateInfo& bufCreateInfo) -> buf {
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

    auto Allocator::copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize, vk::CommandBuffer& cmd) -> void {
        vk::BufferCopy copy{0, 0, deviceSize};
        cmd.copyBuffer((vk::Buffer) stagingBuf, (vk::Buffer) destBuf, 1, &copy);
    }

    auto Allocator::mapBuf(const yic::Allocator::buf& buf, VkDeviceSize devSize, const void *data, bool unmap)-> void* {
        void* mappedData = nullptr;

        try {
            if (data == nullptr){
                mappedData = buf.vmaAllocation->GetMappedData();
            }
            if (!unmap && data != nullptr) {
                mappedData = buf.vmaAllocation->GetMappedData();
                memcpy(mappedData, data, devSize);
            } else if (unmap) {
               // vmaMapMemory(mVmaAllocator, buf.vmaAllocation, &mappedData);
                mappedData = buf.vmaAllocation->GetMappedData();
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

    auto Allocator::copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h, vk::CommandBuffer &cmd) -> void {
        transitionImageLayout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal, cmd);

        vk::BufferImageCopy region{
                0, 0, 0,
                vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1},
                {0, 0, 0}, {w, h, 1}
        };

        cmd.copyBufferToImage(buf, img, vk::ImageLayout::eTransferDstOptimal, region);

        transitionImageLayout(img, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
    }


    auto Allocator::transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout,
                                            vk::ImageLayout newLayout, vk::CommandBuffer& cmd) -> void {

        vk::ImageMemoryBarrier barrier{{}, {}, oldLayout, newLayout,
                                       vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
                                       vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        vk::PipelineStageFlags srcStage, dstStage;
        if (oldLayout == vk::ImageLayout::eUndefined) {
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

        cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);

    }

    auto Allocator::resetBuf(Allocator::stagBuf& buf, vk::CommandBuffer& cmd) -> void {
        vk::MemoryBarrier copyBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead};
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, copyBarrier, {}, {});
        cmd.fillBuffer(buf.stgBuf.buf, 0, VK_WHOLE_SIZE, 0);
    }

    auto Allocator::acquireStagingBuf(vk::DeviceSize deviceSize) -> stagBuf {
        auto it = mStagingBufs.lower_bound(deviceSize);
        while(it != mStagingBufs.end()){
            stagBuf stgBuf;
            while (it->second.try_pop(stgBuf)){
                return stgBuf;
            }
            it++;
        }
        auto buf = createBuf({
                                     .deviceSize = deviceSize,
                                     .usage = vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                                     .memoryUsage = MemoryUsage::eCpuOnly,
                                     .allocStrategy = AllocStrategy::eMapped,});
        mStagBufCounter++;
        stagBuf stgBuf{buf, deviceSize};
        return stgBuf;
    }

    auto Allocator::releaseStagingBuf(yic::Allocator::stagBuf stg) -> void {
        mStagingBufs[stg.deviceSize].push(stg);
    }

    auto Allocator::destroyStagBuf() -> void {
        for(auto& [key, stg] : mStagingBufs){
            stagBuf stgBuf;
            while(stg.try_pop(stgBuf)){
                vmaDestroyBuffer(mVmaAllocator, stgBuf.stgBuf.buf, stgBuf.stgBuf.vmaAllocation);
                mDestroyCount++;
            }
        }
        if_debug {
            if (mStagBufCounter != mDestroyCount) {
                vkError("Stg buf: {0} {1}", mStagBufCounter, mDestroyCount);
            } else { vkInfo("Stg buf: {0}", mStagBufCounter); }
        }
        mStagingBufs.clear();
    }

        auto
        Allocator::buildBLAS_impl(const std::vector<BLASInput> &input, vk::BuildAccelerationStructureFlagsKHR flags) -> std::vector<vkAccel_sptr> {
        auto size = input.size();
        vk::DeviceSize asTotalSize{}, maxScratchSize{};
        uint32_t compactions{};

        std::vector<BuildAccelerationStructure> buildAS(size);

        for(size_t i = 0; i < size; i++){
            auto& build = buildAS[i];
            auto& blas = input[i];

            build.geoInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                            .setMode(vk::BuildAccelerationStructureModeKHR::eBuild)
                            .setFlags(blas.flags | flags)
                            .setGeometries(blas.asGeometry);

            build.rangeInfo = blas.asBuildRangeInfo.data();

            std::vector<uint32_t> maxPrimitiveCount( blas.asBuildRangeInfo.size() );
            for(auto t = 0; t < blas.asBuildRangeInfo.size(); t++){
                maxPrimitiveCount[t] = blas.asBuildRangeInfo[t].primitiveCount;
            }

            build.sizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                          build.geoInfo, maxPrimitiveCount, mDyDispatcher);

            asTotalSize += build.sizeInfo.accelerationStructureSize;
            maxScratchSize = std::max(maxScratchSize, build.sizeInfo.buildScratchSize);
            compactions += hasFlag(build.geoInfo.flags, vk::BuildAccelerationStructureFlagBitsKHR::eAllowCompaction);
        }

        auto scratchBuf = Allocator::allocBufStaging(maxScratchSize, vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eStorageBuffer);
        vk::BufferDeviceAddressInfo bufferInfo{scratchBuf->buffer};
        vk::DeviceAddress  scratchAddress = mDevice.getBufferAddress(bufferInfo);

        std::vector<uint32_t> indices{};
        vk::DeviceSize batchSize{};
        vk::DeviceSize batchLimit{256'000'000};

        for(auto i = 0; i < size; i++){
            indices.emplace_back(i);
            batchSize += buildAS[i].sizeInfo.accelerationStructureSize;

            if (batchSize >= batchLimit || i == size - 1){
                for(const auto& index : indices){
                    auto &build = buildAS[index];
                    auto createInfo = vk::AccelerationStructureCreateInfoKHR()
                            .setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
                            .setSize(build.sizeInfo.accelerationStructureSize);

                    build.asSptr = allocAccel(createInfo);
                    build.geoInfo.setDstAccelerationStructure(build.asSptr->accel)
                            .setScratchData(scratchAddress);

                    CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer &cmd) {
                        cmd.buildAccelerationStructuresKHR(build.geoInfo, build.rangeInfo,
                                                           mDyDispatcher);

                        vk::MemoryBarrier barrier{vk::AccessFlagBits::eAccelerationStructureWriteKHR,
                                                  vk::AccessFlagBits::eAccelerationStructureReadKHR};
                        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                            vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
                                            {}, barrier, {}, {});
                    });
                }

                batchSize = 0;
                indices.clear();
            }
        }

        std::vector<vkAccel_sptr> as{};
        for(auto& build : buildAS){
            as.emplace_back(build.asSptr);
        }

        return as;
    }

    auto Allocator::buildTLAS_impl(const std::vector<vk::AccelerationStructureInstanceKHR> &instances,
                                   vk::BuildAccelerationStructureFlagsKHR flags,
                                   bool update) -> vkAccel_sptr {
        uint32_t size = instances.size();

        auto instBuf = Allocator::allocBufStaging(sizeof(vk::AccelerationStructureInstanceKHR) * size, instances.data(),
                                                  vk::BufferUsageFlagBits::eShaderDeviceAddress |
                                                  vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR |
                                                  vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR);

        vk::BufferDeviceAddressInfo addrInfo{instBuf->buffer};
        vk::DeviceSize addrSize(mDevice.getBufferAddress(addrInfo, mDyDispatcher));

        vk::AccelerationStructureGeometryInstancesDataKHR geoInstData{{}, addrSize};
        vk::AccelerationStructureGeometryKHR tlasGeo{vk::GeometryTypeKHR::eInstances, geoInstData};

        vk::AccelerationStructureBuildGeometryInfoKHR info{
                vk::AccelerationStructureTypeKHR::eTopLevel, flags,
                update ? vk::BuildAccelerationStructureModeKHR::eUpdate : vk::BuildAccelerationStructureModeKHR::eBuild,
                {}, {}, tlasGeo, {}
        };

        auto sizeInfo = mDevice.getAccelerationStructureBuildSizesKHR(vk::AccelerationStructureBuildTypeKHR::eDevice,
                                                                      info, size, mDyDispatcher);

#ifdef  VK_NV_ray_tracing_motion_blur
        vk::AccelerationStructureMotionInfoNV motionInfoNv{size};
#endif

        vkAccel_sptr tlas;
        if (!update) {
            auto createInfo = vk::AccelerationStructureCreateInfoKHR()
                    .setType(vk::AccelerationStructureTypeKHR::eTopLevel)
                    .setSize(sizeInfo.accelerationStructureSize);

            tlas = Allocator::allocAccel(createInfo);
        }

        auto scratchBuf = Allocator::allocBufStaging(sizeInfo.buildScratchSize,
                                                     vk::BufferUsageFlagBits::eStorageBuffer |
                                                     vk::BufferUsageFlagBits::eShaderDeviceAddress);

        vk::BufferDeviceAddressInfo bufAddrInfo{scratchBuf->buffer};
        vk::DeviceSize scratchAddr = mDevice.getBufferAddress(bufAddrInfo, mDyDispatcher);

        info.setSrcAccelerationStructure(update ? tlas->accel : VK_NULL_HANDLE)
                .setDstAccelerationStructure(tlas->accel)
                .setScratchData(scratchAddr);

        vk::AccelerationStructureBuildRangeInfoKHR rangeInfoKhr{size, 0, 0, 0};

        CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer &cmd) {
            vk::MemoryBarrier barrier{
                    vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead
            };
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, {}, barrier, {}, {});
            cmd.buildAccelerationStructuresKHR(info, &rangeInfoKhr, mDyDispatcher);
        });

        return tlas;
    }

    auto Allocator::allocAccel_impl(vk::AccelerationStructureCreateInfoKHR &createInfo) -> vkAccel_sptr{
        bufCreateInfo bufCreateInfo{
                .deviceSize = createInfo.size,
                .usage = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,

                .memoryUsage = MemoryUsage::eGpuOnly,
                .allocStrategy = AllocStrategy::eDefault,
        };
        auto buf = createBuf(bufCreateInfo);
        createInfo.buffer = buf.buf;
        auto as = mDevice.createAccelerationStructureKHR(createInfo, nullptr, mDyDispatcher);
        return std::make_shared<vkAccel>(buf.buf, buf.vmaAllocation, mVmaAllocator, as, mDevice, mDyDispatcher);
    }

//    auto Allocator::modelToGeometryKHR_impl(sc::Model::Generic &model) -> BLASInput {
//        BLASInput input;
//        for(auto& mesh : model.meshes){
//            auto vertAddr = getBufAddr(mesh.vertBuf);
//            auto indexAddr = getBufAddr(mesh.indexBuf);
//
//            model.meshesAddr.emplace_back(vertAddr, indexAddr);
//
//            auto maxPrimitiveCount = mesh.indices.size() / 3;
//
//            auto tri = vk::AccelerationStructureGeometryTrianglesDataKHR()
//                    .setVertexFormat(vk::Format::eR32G32B32Sfloat)
//                    .setVertexStride(sizeof(sc::Vertex))
//                    .setVertexData(vertAddr)
//                    .setMaxVertex(mesh.vertices.size())
//                    .setIndexType(vk::IndexType::eUint32)
//                    .setIndexData(indexAddr);
//            auto asGeo = vk::AccelerationStructureGeometryKHR()
//                    .setGeometryType(vk::GeometryTypeKHR::eTriangles)
//                    .setGeometry(tri)
//                    .setFlags(vk::GeometryFlagBitsKHR::eOpaque);
//            auto range = vk::AccelerationStructureBuildRangeInfoKHR()
//                    .setPrimitiveCount(maxPrimitiveCount)
//                    .setFirstVertex(0)
//                    .setPrimitiveOffset(0)
//                    .setTransformOffset(0);
//
//            input.asGeometry.emplace_back(asGeo);
//            input.asBuildRangeInfo.emplace_back(range);
//        }
//        model.meshesAddrBuf = allocBufStaging(model.meshesAddr.size() * sizeof (sc::MeshBufAddress), model.meshesAddr.data(), vk::BufferUsageFlagBits::eStorageBuffer);
//
//        return input;
//    }

    auto Allocator::getBufAddr(const vkBuf_sptr& buf) -> vk::DeviceAddress{
        if (!buf){
            return 0;
        }

        return get()->mDevice.getBufferAddress(buf->buffer);
    }


} // yic