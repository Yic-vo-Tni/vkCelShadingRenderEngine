//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_TIMELINE_H
#define VKCELSHADINGRENDERER_TIMELINE_H

#include "RHI/GpuRuntime/Alloctor/ThreadSafeLRUCache.h"

namespace rhi2 {

    class Allocator2 {
    public:
        Allocator2();
        ~Allocator2();
        MAKE_SINGLETON(Allocator2);

        template<typename T>
        auto update(const handle::buffer& handle, const T& src, const vk::DeviceSize& offset = 0) -> void {
            updateT(handle, &src, sizeof(T), offset);
        }
        template<typename T>
        auto update(const handle::buffer& handle, const vot::vector<T>& src, const vk::DeviceSize& offset = 0) -> void {
            if (!src.empty())
                updateT(handle, src.data(), src.size() * sizeof(T), offset);
        }
        template<typename T>
        auto update(const handle::buffer& handle, const std::pmr::vector<T>& src, const vk::DeviceSize& offset = 0) -> void {
            if (!src.empty())
                updateT(handle, src.data(), src.size() * sizeof(T), offset);
        }

        auto allocBuffer(BufferAttachment bufferCI) -> handle::buffer;
        auto unLoad(const handle::buffer& handle) -> void;
    private:
        auto updateT(const handle::buffer& handle, const void* data, const vk::DeviceSize& size, const vk::DeviceSize& offset = 0) -> void;
        auto allocBufferMin(const BufferAttachment& ci) -> handle::buffer;
        auto allocBufferMax(BufferAttachment& ci) -> handle::buffer;
        [[nodiscard]] auto createBuffer(const BufferAttachment& ci) const -> std::shared_ptr<BufferMata>;
        auto mapBufferPersistent(const BufferMata &bufferMata, const void* data, vk::DeviceSize size) -> void*;
        auto updateTransient(const BufferMata &bufferMata, const void* data, vk::DeviceSize size, const bool unmap) const -> void*;
        auto copyBuffer(const BufferMata& src, const BufferMata& dst, const vk::DeviceSize& size, const vot::CommandBuffer& cmd) -> void;
        auto resetBuffer(const BufferMata& bufferMata, vot::CommandBuffer& cmd) -> void;
        [[nodiscard]] auto pickPool(const BufferAttachment& attach)  const -> VmaPool;

        auto acquireBufferCache(const vk::DeviceSize& size) -> std::shared_ptr<BufferMata>;
        // auto releaseStagingBuffer(const std::shared_ptr<BufferMata>& bufferMata) -> void;

        ev::pVkSetupContext ct{};
        VmaAllocator mVmaAllocator{};

        VmaPool mTransientPool{};
        VmaPool mStaticMaxPool{};
        VmaPool mStaticMinPool{};

        oneapi::tbb::concurrent_vector<std::shared_ptr<BufferMata>> mBuffers;
        // std::atomic<uint8_t> mStagBufferCounter{};
        // std::atomic<uint8_t> mDestroyCount{};
        // oneapi::tbb::concurrent_map<vk::DeviceSize , oneapi::tbb::concurrent_queue<std::shared_ptr<BufferMata>>> mStagingBuffers;
        vot::gfx::ThreadSafeLRUCache<vk::DeviceSize, std::shared_ptr<BufferMata>> mBufferCaches;
        // oneapi::tbb::concurrent_lru_cache<vk::DeviceSize, std::shared_ptr<BufferMata>> mBufferCaches;
    };
} // rhi2

namespace yic {
    inline rhi2::Allocator2* allocator2;
}



#endif //VKCELSHADINGRENDERER_TIMELINE_H