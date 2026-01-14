//
// Created by lenovo on 1/14/2026.
//

#ifndef VKCELSHADINGRENDERER_ALLOCATORTYPES_H
#define VKCELSHADINGRENDERER_ALLOCATORTYPES_H

namespace vot::gfx {
    namespace detail {
        using BufferAllocation = std::pair<VkBuffer, VmaAllocation>;
        using ImageAllocation = std::pair<VkImage, VmaAllocation>;
        using StagingBufferAllocation = std::tuple<VkBuffer, VmaAllocation, vk::DeviceSize>;

    }

    namespace api {
        enum class MemoryUsage : std::uint32_t {
            eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
            eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
            eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
            eGpuToCPU = VMA_MEMORY_USAGE_GPU_TO_CPU,
        };

        enum class AllocStrategy : std::uint32_t {
            eDefault = 0,
            eMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
            eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            eHostSequentialWrite = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
            eDedicated = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        };

        enum class BufferResidency : std::uint32_t {
            eDefault, eStaging, eDedicated, eCount
        };

        struct BufferCI {
            string id = IdGenerator::uniqueId();
            BufferResidency residency = BufferResidency::eDefault;
            const void* data = nullptr;
            vk::DeviceSize device_size;
            vk::BufferUsageFlags buffer_usage_flags;
            MemoryUsage memory_usage;
            AllocStrategy alloc_strategy = AllocStrategy::eMapped;

            auto &setId(string newId) { id = std::move(newId); return *this; }
            auto &setData(const void *ptr) { data = ptr; return *this; }
            auto &setDeviceSize(const vk::DeviceSize size) { device_size = size; return *this; }
            auto &setUsage(const vk::BufferUsageFlags usage) { buffer_usage_flags = usage; return *this; }
            auto &addUsage(const vk::BufferUsageFlags usage) { buffer_usage_flags |= usage; return *this; }
            auto &setMemoryUsage(const MemoryUsage usage) { memory_usage = usage; return *this; }
            auto &setAllocStrategy(const AllocStrategy strategy) { alloc_strategy = strategy; return *this; }
            auto &setBufferResidency(const BufferResidency buffer_residency) { residency = buffer_residency; return *this; }
        };
    }

}

template<>
struct vot::enable_bitmask_operators<vot::gfx::api::AllocStrategy> : std::true_type {};
template<>
struct entt::enum_as_bitmask<vot::gfx::api::AllocStrategy> : std::true_type {};

#endif //VKCELSHADINGRENDERER_ALLOCATORTYPES_H