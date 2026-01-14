//
// Created by lenovo on 1/14/2026.
//

#include "BufferAllocator.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Buffer.h"
#include "RHI/Command.h"

#include "vma/vk_mem_alloc.h"

namespace vot::gfx {
    BufferAllocator::BufferAllocator(VmaAllocator &vma_allocator, LRUStagingBufferCache &caches_) : vmaA(vma_allocator), caches(caches_) {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
    }

    BufferAllocator::~BufferAllocator() = default;

    auto BufferAllocator::create(const api::BufferCI &ci) -> Buffer_sptr {
       // if (ci.is_staging) {
            return createDeviceLocalBufferWithStaging(ci);
        //}
        return createHostVisibleBuffer(ci);
    }

    auto BufferAllocator::createHostVisibleBuffer(const api::BufferCI &ci) const -> Buffer_sptr {
        auto [fst, snd] = allocateBuffer(ci);
        auto mapped = mapBuffer(snd, ci.device_size, ci.data);

        return std::make_shared<Buffer>(fst, snd, mapped, vmaA, [this, mapped, snd, ci](const void* src) { // HACK:
            memcpy(mapped, src, ci.device_size);
            vmaFlushAllocation(vmaA, snd, 0, ci.device_size);
        }, ci.id);
    }

    auto BufferAllocator::createDeviceLocalBufferWithStaging(api::BufferCI ci) -> Buffer_sptr {
        ci.buffer_usage_flags |= vk::BufferUsageFlagBits::eTransferDst;

        auto [buf, alloc] = allocateBuffer(ci);

        auto uploadViaStaging = [&](const vk::DeviceSize size, const void* src) {
            auto allocation = caches.acquire(size);
            auto& [stg, stgAlloc, s] = allocation;

            mapBuffer(stgAlloc, size, src);
         //   vmaUnmapMemory(vmaA, stgAlloc); // HACK:staging buffer currently uses map/unmap per upload.This avoids persistent mapping state in LRU cache.Can be upgraded to persistent-mapped staging cache later.

            yic::command->drawOneTimeSubmit([&](const CommandBuffer &cmd) {
                copyBuffer(stg, buf, size, cmd);
                resetBuffer(stg, cmd);
            });
        };
        if (ci.data) uploadViaStaging(ci.device_size, ci.data);

        auto upload = [this, buf, size = ci.device_size, uploadViaStaging](const void* src){ uploadViaStaging(size, src); };

        return std::make_shared<Buffer>(buf, alloc, nullptr, vmaA, std::move(upload), ci.id);

        // ci.buffer_usage_flags |= vk::BufferUsageFlagBits::eTransferDst;
        // if (ci.data == nullptr) {
        //     auto [buf, alloc] = allocateBuffer(ci);
        //
        //     return std::make_shared<vot::Buffer>(buf, alloc, nullptr, vmaA, [this, ci, buf](const void *src) {
        //         auto stagingBufferHandle = caches.acquire(ci.device_size);
        //         auto &[stgBuf, stagAlloc, s] = stagingBufferHandle;
        //
        //         mapBuffer(stagAlloc, ci.device_size, src);
        //
        //         yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
        //             copyBuffer(stgBuf, buf, ci.device_size, cmd);
        //             resetBuffer(stgBuf, cmd);
        //         });
        //
        //     }, ci.id);
        // }
        // auto stagingBufferHandle = caches.acquire(ci.device_size);
        // auto &[stagingBuffer, stagingAlloc, size] = stagingBufferHandle;
        //
        // mapBuffer(stagingAlloc, ci.device_size, ci.data);
        //
        // auto [buf, alloc] = allocateBuffer(ci);
        //
        // yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
        //     copyBuffer(stagingBuffer, buf, ci.device_size, cmd);
        //     resetBuffer(stagingBuffer, cmd);
        // });
        //
        // return std::make_shared<vot::Buffer>(buf, alloc, nullptr, vmaA, [this, ci, buf](const void *src) {
        //     auto stagingBufferHandle = caches.acquire(ci.device_size);
        //     auto &[stgBuf, stagAlloc, s] = stagingBufferHandle;
        //
        //     mapBuffer(stagAlloc, ci.device_size, src);
        //
        //     yic::command->drawOneTimeSubmit([&](vot::CommandBuffer &cmd) {
        //         copyBuffer(stgBuf, buf, ci.device_size, cmd);
        //         resetBuffer(stgBuf, cmd);
        //     });
        //
        // }, ci.id);
    }

    auto BufferAllocator::allocateBuffer(const api::BufferCI& ci) const -> detail::BufferAllocation {
        detail::BufferAllocation buffer_allocation;

        const VkBufferCreateInfo bufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = ci.device_size,
            .usage = static_cast<VkBufferUsageFlags>(ci.buffer_usage_flags),
        };
        const VmaAllocationCreateInfo vmaCI{
            .flags = static_cast<VmaAllocationCreateFlags>(ci.alloc_strategy),
            .usage = static_cast<VmaMemoryUsage>(ci.memory_usage),
        };

        if (vmaCreateBuffer(vmaA, &bufferCI, &vmaCI, &buffer_allocation.first, &buffer_allocation.second, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer");
        }

        return buffer_allocation;
    }

    auto BufferAllocator::mapBuffer(const VmaAllocation &alloc, const VkDeviceSize devSize, const void *data) const -> void * {
        void *mapped = nullptr;

        // if (const VkResult res = vmaMapMemory(vmaA, alloc, &mapped); res != VK_SUCCESS || mapped == nullptr) {
        //   throw std::runtime_error("failed to map buffer");
        // }
        //
        // if (data) {
        //     std::memcpy(mapped, data, devSize);
        //     vmaFlushAllocation(vmaA, alloc, 0, devSize);
        // }

        VmaAllocationInfo info{};
        vmaGetAllocationInfo(vmaA, alloc, &info);
        mapped = info.pMappedData;

        if (!mapped) {
            if (const VkResult res = vmaMapMemory(vmaA, alloc, &mapped); res != VK_SUCCESS || mapped == nullptr) {
              throw std::runtime_error("failed to map buffer");
            }
        }

        if (data) {
            std::memcpy(mapped, data, devSize);
            vmaFlushAllocation(vmaA, alloc, 0, devSize);
        }

        return mapped;
    }

    auto BufferAllocator::copyBuffer(VkBuffer stagingBuffer, VkBuffer destBuffer, const VkDeviceSize deviceSize, const CommandBuffer &cmd) -> void {
        const vk::BufferCopy copy{0, 0, deviceSize};
        cmd.copyBuffer((vk::Buffer)stagingBuffer, (vk::Buffer)destBuffer, copy);
    }

    auto BufferAllocator::resetBuffer(const VkBuffer&buffer, const CommandBuffer &cmd) -> void {
        constexpr vk::MemoryBarrier copyBarrier{vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eTransferRead};
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, copyBarrier, {}, {});
        cmd.fillBuffer(buffer, 0, VK_WHOLE_SIZE, 0);
    }
}
