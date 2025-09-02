//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_TIMELINE_H
#define VKCELSHADINGRENDERER_TIMELINE_H

namespace rhi2 {
    enum class VmaPoolType {
        eTransient,
        eStaticAuto,
        eStaticMax,
        eStaticMin,
        eAuto,
    };

    struct BufferAttachment {
        enum Type { eAuto, eForceHost, eForceStaging };

        Type type{eAuto};
        vk::DeviceSize size{0};
        vk::BufferUsageFlags usage{};
        VmaPoolType vmaPoolType{VmaPoolType::eAuto};
        vot::memoryUsage memoryUsage = vot::memoryUsage::eCpuToGpu;
        vot::allocStrategy allocStrategy = vot::eMapped;
        bool unmap = false;

        void* data;
        vot::string debugName;

        auto setDeviceSize(const vk::DeviceSize& size_) { this->size = size_;  return *this; }
        auto setBufferUsageFlags(const vk::BufferUsageFlags flags)  { usage = flags; return *this; }

        auto optBufferType(const Type type_) { type = type_; return *this; }
        auto optMemoryUsage(const vot::memoryUsage& usage_)   { this->memoryUsage = usage_; return *this; }
        auto optAllocStrategy(const vot::allocStrategy strategy)  { this->allocStrategy = strategy; return *this; }
        auto addAllocStrategy(const vot::allocStrategy strategy)  { this->allocStrategy | strategy; return *this; }
        auto optUnmap(const bool unmap_) { this->unmap = unmap_; return *this;}
        auto optData(void* data_) { this->data = data_; return *this;}
        auto optDebugName(const vot::string& name) { this->debugName = name; return *this;}
    };

    struct BufferMata {
        vk::Buffer buffer{};
        VmaAllocation allocation{};
        void *mapped{nullptr};

        BufferAttachment attach{};
        vot::u32 index{0};
        vot::u32 generation{0};
    };

    struct BufferHandle {
        BufferMata* mata;
        uint32_t generation;

        operator vk::Buffer() const { return mata ? mata->buffer : VK_NULL_HANDLE; }
        operator VmaAllocation() const { return mata ? mata->allocation : nullptr; }
        operator void*() const { return mata ? mata->mapped : nullptr; }

        auto gBuffer() const -> vk::Buffer { return mata ? mata->buffer : VK_NULL_HANDLE; }
        auto gVmaAlloc() const -> VmaAllocation { return mata ? mata->allocation : nullptr; }
        auto gMapped() const -> void * { return mata ? mata->mapped : nullptr; }

        bool valid() const { return mata && mata->generation == generation; }
    };

    class Allocator {
    public:
        Make = []{ return Singleton<Allocator>::make_ptr(); };
        Allocator();
        ~Allocator();

        template<typename T>
        auto update(const BufferHandle& handle, const T& src, const vk::DeviceSize& offset = 0) -> void {
            updateT(handle, &src, sizeof(T), offset);
        }
        template<typename T>
        auto update(const BufferHandle& handle, const vot::vector<T>& src, const vk::DeviceSize& offset = 0) -> void {
            if (!src.empty())
                updateT(handle, src.data(), src.size() * sizeof(T), offset);
        }
        template<typename T>
        auto update(const BufferHandle& handle, const std::pmr::vector<T>& src, const vk::DeviceSize& offset = 0) -> void {
            if (!src.empty())
                updateT(handle, src.data(), src.size() * sizeof(T), offset);
        }

        auto allocBuffer(BufferAttachment bufferCI) -> BufferHandle;
        auto unLoad(const BufferHandle& handle) -> void;
    private:
        auto updateT(const BufferHandle& handle, const void* data, const vk::DeviceSize& size, const vk::DeviceSize& offset = 0) -> void;
        auto allocBufferMin(const BufferAttachment& ci) -> BufferHandle;
        auto allocBufferMax(BufferAttachment& ci) -> BufferHandle;
        auto createBuffer(const BufferAttachment& ci) const -> std::shared_ptr<BufferMata>;
        auto mapBufferPersistent(const BufferMata &bufferMata, const void* data, const vk::DeviceSize size) const -> void*;
        auto updateTransient(const BufferMata &bufferMata, const void* data, const vk::DeviceSize size, const bool unmap) const -> void*;
        auto copyBuffer(const BufferMata& src, const BufferMata& dst, const vk::DeviceSize& size, vot::CommandBuffer& cmd) -> void;
        auto resetBuffer(const BufferMata& bufferMata, vot::CommandBuffer& cmd) -> void;
        auto pickPool(const BufferAttachment& attach)  const -> VmaPool;

        auto acquireStagingBuffer(const vk::DeviceSize& size) -> BufferMata;
        auto releaseStagingBuffer(const BufferMata& bufferMata) -> void;

        ev::pVkSetupContext ct{};
        VmaAllocator mVmaAllocator{};

        VmaPool mTransientPool{};
        VmaPool mStaticMaxPool{};
        VmaPool mStaticMinPool{};

        oneapi::tbb::concurrent_vector<std::shared_ptr<BufferMata>> mBuffers;
    };
} // rhi2

namespace yic {
    inline rhi2::Allocator* allocator2;
}



#endif //VKCELSHADINGRENDERER_TIMELINE_H