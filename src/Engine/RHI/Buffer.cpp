//
// Created by lenovo on 10/4/2024.
//

#include "Buffer.h"
#include "Core/DispatchSystem/SystemHub.h"
#include <utility>

namespace vot::inline rhi {
    Buffer::Buffer(const vk::Buffer buffer, const VmaAllocation alloc, void *data, VmaAllocator &vmaAllocator,
                   const vot::string &id) : Identifiable(id), buffer(buffer), allocation(alloc),
                                            mapped(data),
                                            allocator(vmaAllocator) {
        device = *yic::systemHub.va<ev::pVkSetupContext>().device;
    }

    Buffer::Buffer(const vk::Buffer buffer, const VmaAllocation alloc, void *data, VmaAllocator &vmaAllocator,
                   std::function<void(const void *)> updateFn, const vot::string &id) : Identifiable(id),
        buffer(buffer),
        allocation(alloc),
        mapped(data),
        allocator(vmaAllocator),
        updateFn(std::move(updateFn)) {
        device = *yic::systemHub.va<ev::pVkSetupContext>().device;
    }

    Buffer::~Buffer() {
        auto raw = static_cast<VkBuffer>(buffer);
        if_debug yic::logger->info("Destroy buffer id: {0}, handle: {1}", id, reinterpret_cast<uint64_t>(raw));
        vmaDestroyBuffer(allocator, buffer, allocation);
    }


    ///


    Accel::Accel(vk::Buffer buf, VmaAllocation alloc, VmaAllocator &allocatorRef, vk::AccelerationStructureKHR accel,
                 vot::string id) : Identifiable(std::move(id)), buffer(buf), vmaAllocation(alloc),
                                   mAllocator(allocatorRef),
                                   accel(accel) {
        device = *yic::systemHub.va<ev::pVkSetupContext>().device;
        dyDispatch = *yic::systemHub.va<ev::pVkSetupContext>().dynamicDispatcher;
    }

    Accel::~Accel() {
        vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
        device.destroy(accel, nullptr, dyDispatch);
    }
} // rhi
