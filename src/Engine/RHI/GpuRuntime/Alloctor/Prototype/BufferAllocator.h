//
// Created by lenovo on 1/14/2026.
//

#ifndef VKCELSHADINGRENDERER_BUFFERALLOCATOR_H
#define VKCELSHADINGRENDERER_BUFFERALLOCATOR_H

#include "../AllocatorTypes.h"
#include "../ThreadSafeLRUCache.h"

namespace vot::gfx {
    class BufferAllocator {
    public:
        explicit BufferAllocator(VmaAllocator& vma_allocator, LRUStagingBufferCache& caches_);
        ~BufferAllocator();

        auto create(const api::BufferCI& ci) -> Buffer_sptr;
    private:
        [[nodiscard]] auto createHostVisibleBuffer(const api::BufferCI& ci) const -> Buffer_sptr;
        auto createDeviceLocalBufferWithStaging(api::BufferCI ci) -> Buffer_sptr;
        [[nodiscard]] auto allocateBuffer(const api::BufferCI& ci) const -> detail::BufferAllocation;
        auto mapBuffer(const VmaAllocation& alloc, VkDeviceSize devSize, const void* data) const -> void*;
        auto copyBuffer(VkBuffer stagingBuffer, VkBuffer destBuffer, VkDeviceSize deviceSize, const CommandBuffer& cmd) -> void;
        auto resetBuffer(const VkBuffer& buffer, const CommandBuffer& cmd) -> void;
    private:
        VmaAllocator& vmaA;
        ev::pVkSetupContext ct{};
        LRUStagingBufferCache& caches;
    };
}

#endif //VKCELSHADINGRENDERER_BUFFERALLOCATOR_H
