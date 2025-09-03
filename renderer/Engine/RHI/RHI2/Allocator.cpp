//
// Created by lenovo on 9/1/2025.


#include "Allocator.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Command.h"

#include "vma/vk_mem_alloc.h"

namespace rhi2 {

    constexpr vk::DeviceSize k256Kb = 256 * 1024;
    constexpr vk::DeviceSize k16Mb = 16 * 1024 * 1024;
    constexpr vk::DeviceSize k64Mb = 64 * 1024 * 1024;
    constexpr vk::DeviceSize k512Mb = 512 * 1024 * 1024;

    Allocator::Allocator() : ct(yic::systemHub.va<ev::pVkSetupContext>()), mBufferCaches(32, [&](const std::shared_ptr<BufferMata>& bufferMata) {

    }) {

        const VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = *ct.physicalDevice,
            .device = *ct.device,
            .instance = *ct.instance,
        };

        vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator);

        VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufInfo.size = 64;
        bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        uint32_t memTypeIndex = 0;
        if (vmaFindMemoryTypeIndexForBufferInfo(mVmaAllocator, &bufInfo, &allocInfo, &memTypeIndex) != VK_SUCCESS) {
            throw std::runtime_error("Failed to find memory type index for transient pool");
        }

        VmaPoolCreateInfo transientInfo{};
        transientInfo.blockSize = k16Mb;
        transientInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        transientInfo.memoryTypeIndex = memTypeIndex;
        vmaCreatePool(mVmaAllocator, &transientInfo, &mTransientPool);

        VmaPoolCreateInfo smallInfo{};
        smallInfo.blockSize = k64Mb;
        smallInfo.memoryTypeIndex = memTypeIndex;
        vmaCreatePool(mVmaAllocator, &smallInfo, &mStaticMinPool);

        VmaPoolCreateInfo bigInfo{};
        bigInfo.blockSize = k512Mb;
        bigInfo.memoryTypeIndex = memTypeIndex;
        vmaCreatePool(mVmaAllocator, &bigInfo, &mStaticMaxPool);
    }

    Allocator::~Allocator() {
        for(const auto& buffer : mBuffers) {
            ct.device->destroy(buffer->buffer);
            vmaFreeMemory(mVmaAllocator, buffer->allocation);
        }
        mBuffers.clear();

        vmaDestroyPool(mVmaAllocator, mTransientPool);
        vmaDestroyPool(mVmaAllocator, mStaticMinPool);
        vmaDestroyPool(mVmaAllocator, mStaticMaxPool);

        vmaDestroyAllocator(mVmaAllocator);
    }

    auto Allocator::allocBuffer(BufferAttachment bufferCI) -> handle::buffer {
        switch (bufferCI.type) {
            case BufferAttachment::eForceHost:    return allocBufferMin(bufferCI);
            case BufferAttachment::eForceStaging: return allocBufferMax(bufferCI);
            case BufferAttachment::eAuto:
                return (bufferCI.size < k256Kb)
                         ? allocBufferMin(bufferCI)
                         : allocBufferMax(bufferCI);
        }
        return allocBufferMax(bufferCI);
    }

    auto Allocator::unLoad(const handle::buffer &handle) -> void {

    }

    auto Allocator::updateT(const handle::buffer& handle, const void* data, const vk::DeviceSize& size, const vk::DeviceSize& offset) -> void {
        if (offset + size > handle.mata->attach.size) { yic::logger->warn("failed to update buffer, the data is over, buffer name:{0}", handle.mata->attach.debugName); return; }

        if (handle.mata->attach.type == BufferAttachment::eForceHost) {
            if (handle.mata->mapped) {
                memcpy(static_cast<char *>(handle.mata->mapped) + offset, data, size);
            } else {
                void *mapped;
                if (vmaMapMemory(mVmaAllocator, handle.mata->allocation, &mapped) != VK_SUCCESS)
                    throw std::runtime_error("Failed to map buffer");
                memcpy(static_cast<char *>(mapped) + offset, data, size);
                vmaUnmapMemory(mVmaAllocator, handle.mata->allocation);
            }
        } else {
            const auto stagingBuffer = acquireBufferCache(size);

            if (data != nullptr) {
                if (stagingBuffer->attach.allocStrategy == vot::eMapped) {
                    stagingBuffer->mapped = mapBufferPersistent(*stagingBuffer, data, size);
                } else {
                    stagingBuffer->mapped = updateTransient(*stagingBuffer, data, size, handle.mata->attach.unmap);
                }
            }

            yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
                copyBuffer(*stagingBuffer, *handle.mata, size, cmd);
                resetBuffer(*stagingBuffer, cmd);
            });
            //releaseStagingBuffer(stagingBuffer);
        }
    }

    auto Allocator::allocBufferMin(const BufferAttachment &attach) -> handle::buffer {
        auto mata = createBuffer(attach);
        mata->attach = attach;
        mata->attach.type = BufferAttachment::eForceHost;

        if (attach.data != nullptr) {
            if (mata->attach.allocStrategy == vot::eMapped) {
                mata->mapped = mapBufferPersistent(*mata, attach.data, attach.size);
            } else {
                mata->mapped = updateTransient(*mata, attach.data, attach.size, attach.unmap);
            }
        }

        const auto handle = handle::buffer{.mata = mata.get()};
        mBuffers.push_back(std::move(mata));

        return handle;
    }

    auto Allocator::allocBufferMax(BufferAttachment &attach) -> handle::buffer {
        attach.usage |= vk::BufferUsageFlagBits::eTransferDst;
        attach.type = BufferAttachment::eForceStaging;

        //auto stagingBuffer = acquireStagingBuffer(attach.size);
        auto stagingBuffer = acquireBufferCache(attach.size);

        if (attach.data != nullptr) {
            if (stagingBuffer->attach.allocStrategy == vot::eMapped) {
                stagingBuffer->mapped = mapBufferPersistent(*stagingBuffer, attach.data, attach.size);
            } else {
                stagingBuffer->mapped = updateTransient(*stagingBuffer, attach.data, attach.size, attach.unmap);
            }
        }
        auto mata = createBuffer(attach);
        mata->attach = attach;

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
            copyBuffer(*stagingBuffer, *mata, attach.size, cmd);
            resetBuffer(*stagingBuffer, cmd);
        });
        // releaseStagingBuffer(stagingBuffer);

        const auto handle = handle::buffer{.mata = mata.get()};
        mBuffers.push_back(std::move(mata));

        return handle;
    }

    auto Allocator::createBuffer(const BufferAttachment &attach) const -> std::shared_ptr<BufferMata> {
        VkBuffer buffer{};
        VmaAllocation allocation{};

        const VkBufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = attach.size,
            .usage = static_cast<VkBufferUsageFlags>(attach.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        const VmaAllocationCreateInfo allocInfo{
            .flags = static_cast<VmaAllocationCreateFlags>(attach.allocStrategy),
            .usage = static_cast<VmaMemoryUsage>(attach.memoryUsage),
            .pool = pickPool(attach),
        };

        if (vmaCreateBuffer(mVmaAllocator, &createInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to create buffer");
        return std::make_shared<BufferMata>(buffer, allocation, nullptr, attach);
    }

    auto Allocator::mapBufferPersistent(const BufferMata &bufferMata, const void* data, const vk::DeviceSize size) -> void * {
        if (bufferMata.mapped) {
            memcpy(static_cast<char *>(bufferMata.mapped), data, size);
        } else {
            void *mapped;
            if (vmaMapMemory(mVmaAllocator, bufferMata.allocation, &mapped) != VK_SUCCESS)
                throw std::runtime_error("Failed to map buffer");
            memcpy(static_cast<char *>(mapped), data, size);
            //vmaUnmapMemory(mVmaAllocator, bufferMata.allocation);

            return mapped;
        }
        // void* mapped = bufferMata.allocation->GetMappedData();
        // if (data) {
        //     memcpy(mapped, data, size);
        // }
        return nullptr;
    }

    auto Allocator::updateTransient(const BufferMata &bufferMata, const void* data, const vk::DeviceSize size, const bool unmap) const -> void * {
        void* mapped = nullptr;

        if (!unmap) {
            if (const VkResult res = vmaMapMemory(mVmaAllocator, bufferMata.allocation, &mapped); res != VK_SUCCESS) {
                throw std::runtime_error("Failed to map buffer, VkResult = " + std::to_string(res));
            }
            if (data) memcpy(mapped, data, size);
        } else {
            vmaUnmapMemory(mVmaAllocator, bufferMata.allocation);
        }

        return mapped;
    }

    auto Allocator::copyBuffer(const BufferMata &src, const BufferMata &dst, const vk::DeviceSize &size,
        const vot::CommandBuffer &cmd) -> void {
        const vk::BufferCopy copy{0, 0, size};
        cmd.copyBuffer((vk::Buffer)src.buffer, (vk::Buffer)dst.buffer, copy);
    }


    auto Allocator::resetBuffer(const BufferMata &bufferMata, vot::CommandBuffer &cmd) -> void {
        constexpr vk::MemoryBarrier copyBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead};
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, copyBarrier, {}, {});
        cmd.fillBuffer(bufferMata.buffer, 0, VK_WHOLE_SIZE, 0);
    }

    auto Allocator::pickPool(const BufferAttachment& attach) const -> VmaPool {
        switch (attach.vmaPoolType) {
            case VmaPoolType::eTransient: return mTransientPool;
            case VmaPoolType::eStaticMin: return mStaticMinPool;
            case VmaPoolType::eStaticMax: return mStaticMaxPool;
            case VmaPoolType::eStaticAuto: {

                if (attach.size < k256Kb) return mStaticMinPool;
                return mStaticMaxPool;
            }
            case VmaPoolType::eAuto: {

                if (attach.size < k256Kb) return mStaticMinPool;
                return mStaticMaxPool;
            }
            default: return VK_NULL_HANDLE;
        }
    }

    auto Allocator::acquireBufferCache(const vk::DeviceSize& size) -> std::shared_ptr<BufferMata> {
        // auto it = mStagingBuffers.lower_bound(size);
        //
        // while(it != mStagingBuffers.end()){
        //     std::shared_ptr<BufferMata> mata;
        //     while(it->second.try_pop(mata)){
        //         return mata;
        //     }
        //     ++it;
        // }
        //
        // auto mata = createBuffer(BufferAttachment()
        //     .setBufferUsageFlags(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst)
        //     .setDeviceSize(size)
        //     .optMemoryUsage(vot::memoryUsage::eCpuOnly)
        //     .optAllocStrategy(vot::allocStrategy::eMapped)
        //     .optBufferType(BufferAttachment::eForceHost));
        // ++mStagBufferCounter;
        // return mata;

        std::shared_ptr<BufferMata> mata;
        if (!mBufferCaches.get(size, mata)) {
            mata = createBuffer(BufferAttachment()
                .setBufferUsageFlags(vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst)
                .setDeviceSize(size)
                .optMemoryUsage(vot::memoryUsage::eCpuOnly)
                .optAllocStrategy(vot::allocStrategy::eMapped)
                .optBufferType(BufferAttachment::eForceHost));
            mBufferCaches.put(size, mata);
        }
        return mata;
    }

    // auto Allocator::releaseStagingBuffer(const std::shared_ptr<BufferMata> &bufferMata) -> void {
    //     // mStagingBuffers[bufferMata->attach.size].push(bufferMata);
    // }
} // rhi2