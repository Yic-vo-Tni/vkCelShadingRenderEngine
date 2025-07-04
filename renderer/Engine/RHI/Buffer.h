//
// Created by lenovo on 10/4/2024.
//

#ifndef VKCELSHADINGRENDERER_BUFFER_H
#define VKCELSHADINGRENDERER_BUFFER_H

namespace vot::inline rhi {

    struct Buffer : public Identifiable{
    public:
        Buffer(vk::Buffer buffer, VmaAllocation alloc, void* data, VmaAllocator& vmaAllocator, const vot::string& id);
        Buffer(vk::Buffer buffer, VmaAllocation alloc, void* data, VmaAllocator& vmaAllocator, std::function<void(const void* scr)> updateFn, const vot::string& id);
        ~Buffer() override;

        [[nodiscard]] auto bufferInfo(vk::DeviceSize offset = 0, vk::DeviceSize range = vk::WholeSize) const{
            return vk::DescriptorBufferInfo{buffer, offset, range};
        }

        template<typename T>
        auto update(const T& src, bool unmap = false){
            updateFn(&src);
        }

        template<typename T>
        auto update(const vot::vector<T>& src, bool unmap = false){
            if (!src.empty())
                updateFn(src.data());
        }

        auto bufferAddr() -> vk::DeviceAddress {
            if (!buffer) return 0;

            return device.getBufferAddress(buffer);
        }
    public:
        vk::Buffer buffer;
        VmaAllocation allocation;
        void* mapped = nullptr;
        VmaAllocator& allocator;
        std::function<void(const void* scr)> updateFn;

    private:
        vk::Device device;
    };

struct Accel : public  Identifiable{
    vk::Buffer buffer{};
    VmaAllocation vmaAllocation{};
    VmaAllocator &mAllocator;
    vk::AccelerationStructureKHR accel;
    bool update{false};

    Accel(vk::Buffer buf, VmaAllocation alloc, VmaAllocator &allocatorRef, vk::AccelerationStructureKHR accel, vot::string id);
    ~Accel() override;

    auto accelAddr() -> vk::DeviceSize {
        vk::AccelerationStructureDeviceAddressInfoKHR addressInfoKhr{accel};
        return device.getAccelerationStructureAddressKHR(addressInfoKhr, dyDispatch);
    }

    [[nodiscard]] auto accelInfo() const {
        return vk::WriteDescriptorSetAccelerationStructureKHR{accel};
    }

private:
    vk::Device device{};
    vk::DispatchLoaderDynamic dyDispatch;
};


} // rhi

#endif //VKCELSHADINGRENDERER_BUFFER_H
