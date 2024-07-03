//
// Created by lenovo on 6/22/2024.
//

#include "vkAllocator.h"

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

        auto transferCmdPoolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(ct.qIndexTransferDownload_v())
                .setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        mTransferPool = ct.device_ref().createCommandPool(transferCmdPoolInfo);
    }

    vkAllocator::~vkAllocator(){
        EventBus::Get::vkSetupContext().device_ref().destroy(mTransferPool);
        vmaDestroyAllocator(mVmaAllocator);
    }


    auto vkAllocator::allocBuf_impl(VkDeviceSize deviceSize, const void *data, VkBufferUsageFlags flags, VmaMemoryUsage usage, bool unmap) -> vkBuf_sptr {
        VkBufferCreateInfo bufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = deviceSize,
                .usage = flags,
        };

        VmaAllocationCreateInfo allocInfo{
                .flags = unmap ? VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT : VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = usage,
        };

        VkBuffer buffer;
        VmaAllocation allocation;
        void* mappedData = nullptr;
        vmaCreateBuffer(mVmaAllocator, &bufferCreateInfo, &allocInfo, &buffer, &allocation, nullptr);

        try {
            if (data == nullptr){
                mappedData = allocation->GetMappedData();
            }
            if (!unmap && data != nullptr) {
                mappedData = allocation->GetMappedData();
                memcpy(mappedData, data, deviceSize);
            } else if (unmap) {
                vmaMapMemory(mVmaAllocator, allocation, &mappedData);
                memcpy(mappedData, data, deviceSize);
                vmaUnmapMemory(mVmaAllocator, allocation);
                mappedData = nullptr;
            }
        } catch (...) {
            vmaDestroyBuffer(mVmaAllocator, buffer, allocation);
            throw;
        }

        return std::make_shared<un::vkBuffer>(buffer, allocation, mappedData, mVmaAllocator);
    }

    auto vkAllocator::allocBuf_staging_impl(VkDeviceSize deviceSize, const void *data, VkBufferUsageFlags flags) -> vkBuf_sptr {
            VkBufferCreateInfo stagingBufferInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = deviceSize,
                    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo stagingAllocInfo{
                    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
                    .usage = VMA_MEMORY_USAGE_CPU_ONLY,
            };

            VkBuffer stagingBuffer;
            VmaAllocation stagingAllocation;
            void* stagingMappedData = nullptr;
            vmaCreateBuffer(mVmaAllocator, &stagingBufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation,
                            nullptr);

            memcpy(stagingMappedData, data, deviceSize);

            VkBufferCreateInfo destBufferInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = deviceSize,
                    .usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo destAllocInfo{
                    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            };

            VkBuffer destBuffer;
            VmaAllocation destAllocation;
            vmaCreateBuffer(mVmaAllocator, &destBufferInfo, &destAllocInfo, &destBuffer, &destAllocation, nullptr);

            copyBuf(stagingBuffer, destBuffer, deviceSize);

            vmaDestroyBuffer(mVmaAllocator, stagingBuffer, stagingAllocation);

            return std::make_shared<un::vkBuffer>(destBuffer, destAllocation, nullptr, mVmaAllocator, [&](){ });
    }

    auto vkAllocator::copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize) -> void {
        createTempCmdBuf();
        vk::BufferCopy copy{0, 0, deviceSize};
        mTempCmd.copyBuffer((vk::Buffer)stagingBuf, (vk::Buffer)destBuf, 1, &copy);
        submitTempCmdBuf();
    }

    auto vkAllocator::createOffScreenImage(vkImageConfig config) -> void {
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
    }




} // yic