//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_RHI2_H
#define VKCELSHADINGRENDERER_RHI2_H

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
        vot::details::u32 index{0};
        vot::details::u32 generation{0};
    };

    namespace handle {
        struct buffer {
            BufferMata *mata;
            vot::details::u32 generation;

            operator vk::Buffer() const { return mata ? mata->buffer : VK_NULL_HANDLE; }
            operator VmaAllocation() const { return mata ? mata->allocation : nullptr; }
            operator void *() const { return mata ? mata->mapped : nullptr; }

            [[nodiscard]] auto gBuffer() const -> vk::Buffer { return mata ? mata->buffer : VK_NULL_HANDLE; }
            [[nodiscard]] auto gVmaAlloc() const -> VmaAllocation { return mata ? mata->allocation : nullptr; }
            [[nodiscard]] auto gMapped() const -> void * { return mata ? mata->mapped : nullptr; }

            [[nodiscard]] bool valid() const { return mata && mata->generation == generation; }
        };
    }

}

#endif //VKCELSHADINGRENDERER_RHI2_H