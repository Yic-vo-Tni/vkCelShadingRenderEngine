//
// Created by lenovo on 10/4/2024.
//

#include "Buffer.h"
#include "Core/DispatchSystem/SystemHub.h"
#include <utility>

namespace vot::inline rhi {

Buffer::Buffer(vk::Buffer buffer, VmaAllocation alloc, void *data, VmaAllocator &vmaAllocator,
               const vot::string &id) : buffer(buffer), allocation(alloc), mapped(data),
                                        allocator(vmaAllocator),
                                        Identifiable(id) {
    device = *yic::systemHub.val<ev::pVkSetupContext>().device;
}

Buffer::Buffer(vk::Buffer buffer, VmaAllocation alloc, void *data, VmaAllocator &vmaAllocator,
               std::function<void(const void *)> updateFn, const vot::string &id) : buffer(buffer), allocation(alloc),
                                                                                    mapped(data),
                                                                                    allocator(vmaAllocator),
                                                                                    updateFn(std::move(updateFn)),
                                                                                    Identifiable(id) {
    device = *yic::systemHub.val<ev::pVkSetupContext>().device;
}

Buffer::~Buffer() {
    std::cout << "desctroy" << id << std::endl;
    vmaDestroyBuffer(allocator, buffer, allocation);
}


///


Accel::Accel(vk::Buffer buf, VmaAllocation alloc, VmaAllocator &allocatorRef, vk::AccelerationStructureKHR accel,
             vot::string id): buffer(buf), vmaAllocation(alloc), mAllocator(allocatorRef), accel(accel),
                              Identifiable(std::move(id)) {
    device = *yic::systemHub.val<ev::pVkSetupContext>().device;
    dyDispatch = *yic::systemHub.val<ev::pVkSetupContext>().dynamicDispatcher;
}

Accel::~Accel() {
    vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
    device.destroy(accel, nullptr, dyDispatch);
}

} // rhi
