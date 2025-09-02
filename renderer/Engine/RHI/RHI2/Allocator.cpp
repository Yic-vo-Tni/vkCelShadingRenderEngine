//
// Created by lenovo on 9/1/2025.
//

#include "Allocator.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Command.h"
#define VMA_DEBUG_DETECT_MEMORY_LEAKS 0
#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

namespace rhi2 {

    constexpr vk::DeviceSize k256Kb = 256 * 1024;
    constexpr vk::DeviceSize k16Mb = 16 * 1024 * 1024;
    constexpr vk::DeviceSize k64Mb = 64 * 1024 * 1024;
    constexpr vk::DeviceSize k512Mb = 512 * 1024 * 1024;

    Allocator::Allocator() {
        ct = yic::systemHub.val<ev::pVkSetupContext>();

        const VmaAllocatorCreateInfo vmaAllocatorCreateInfo{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = *ct.physicalDevice,
            .device = *ct.device,
            .instance = *ct.instance,
        };

        vmaCreateAllocator(&vmaAllocatorCreateInfo, &mVmaAllocator);

        VmaPoolCreateInfo transientInfo{};
        transientInfo.blockSize = k16Mb;
        transientInfo.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT;
        vmaCreatePool(mVmaAllocator, &transientInfo, &mTransientPool);

        VmaPoolCreateInfo smallInfo{};
        smallInfo.blockSize = k64Mb;
        vmaCreatePool(mVmaAllocator, &smallInfo, &mStaticMinPool);

        VmaPoolCreateInfo bigInfo{};
        bigInfo.blockSize = k512Mb;
        vmaCreatePool(mVmaAllocator, &bigInfo, &mStaticMaxPool);
    }

    Allocator::~Allocator() {
        for(const auto& buffer : mBuffers) {
            ct.device->destroy(buffer->buffer);
            vmaFreeMemory(mVmaAllocator, buffer->allocation);
        }
        mBuffers.clear();
        vmaDestroyAllocator(mVmaAllocator);
    }

    auto Allocator::allocBuffer(BufferAttachment bufferCI) -> BufferHandle {
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

    auto Allocator::unLoad(const BufferHandle &handle) -> void {

    }

    auto Allocator::updateT(const BufferHandle& handle, const void* data, const vk::DeviceSize& size, const vk::DeviceSize& offset) -> void {
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
            auto stagingBuffer = acquireStagingBuffer(size);

            if (data != nullptr) {
                if (stagingBuffer.attach.allocStrategy == vot::eMapped) {
                    stagingBuffer.mapped = mapBufferPersistent(stagingBuffer, data, size);
                } else {
                    stagingBuffer.mapped = updateTransient(stagingBuffer, data, size, handle.mata->attach.unmap);
                }
            }

            yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
                copyBuffer(stagingBuffer, *handle.mata, size, cmd);
                resetBuffer(stagingBuffer, cmd);
            });
            releaseStagingBuffer(stagingBuffer);
        }
    }

    auto Allocator::allocBufferMin(const BufferAttachment &ci) -> BufferHandle {
        auto mata = createBuffer(ci);
        mata->attach = ci;
        mata->attach.type = BufferAttachment::eForceHost;

        if (ci.data != nullptr) {
            if (mata->attach.allocStrategy == vot::eMapped) {
                mata->mapped = mapBufferPersistent(*mata, ci.data, ci.size);
            } else {
                mata->mapped = updateTransient(*mata, ci.data, ci.size, ci.unmap);
            }
        }

        const auto handle = BufferHandle{.mata = mata.get()};
        mBuffers.push_back(std::move(mata));

        return handle;
    }

    auto Allocator::allocBufferMax(BufferAttachment &ci) -> BufferHandle {
        ci.usage |= vk::BufferUsageFlagBits::eTransferDst;
        ci.type = BufferAttachment::eForceStaging;

        auto stagingBuffer = acquireStagingBuffer(ci.size);

        if (ci.data != nullptr) {
            if (stagingBuffer.attach.allocStrategy == vot::eMapped) {
                stagingBuffer.mapped = mapBufferPersistent(stagingBuffer, ci.data, ci.size);
            } else {
                stagingBuffer.mapped = updateTransient(stagingBuffer, ci.data, ci.size, ci.unmap);
            }
        }
        auto mata = createBuffer(ci);
        mata->attach = ci;

        yic::command->drawOneTimeSubmit([&](vot::CommandBuffer& cmd) {
            copyBuffer(stagingBuffer, *mata, ci.size, cmd);
            resetBuffer(stagingBuffer, cmd);
        });
        releaseStagingBuffer(stagingBuffer);

        const auto handle = BufferHandle{.mata = mata.get()};
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

    auto Allocator::mapBufferPersistent(const BufferMata &bufferMata, const void* data, const vk::DeviceSize size) const -> void * {
        void* mapped = bufferMata.allocation->GetMappedData();
        if (data) {
            memcpy(mapped, data, size);
        }
        return mapped;
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

    auto Allocator::pickPool(const BufferAttachment& attach) const -> VmaPool {
        switch (attach.vmaPoolType) {
            case VmaPoolType::eTransient: return mTransientPool;
            case VmaPoolType::eStaticMin: return mStaticMinPool;
            case VmaPoolType::eStaticMax: return mStaticMaxPool;
            case VmaPoolType::eStaticAuto: {

                if (attach.size < k256Kb) return mStaticMinPool;
                else return mStaticMaxPool;
            }
            case VmaPoolType::eAuto: {

                if (attach.size < k256Kb) return mStaticMinPool;
                else return mStaticMaxPool;
            }
            default: return VK_NULL_HANDLE;
        }
    }


} // rhi2