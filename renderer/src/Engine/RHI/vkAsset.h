#include <utility>

//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKBUF_H
#define VKCELSHADINGRENDERER_VKBUF_H

namespace yic {

    struct vkAsset{
        enum class Type{
            eBuffer, eImage
        };
    protected:
        Type mType;

        explicit vkAsset(Type type) : mType(type){}
    };

    struct vkBuffer : vkAsset {
        vk::Buffer buffer;
        VmaAllocation vmaAllocation;
        void *mappedData = nullptr;
        VmaAllocator &mAllocator;
        std::function<void(const void* src)> mUpdateStagingFunc;

        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef), vkAsset(vkAsset::Type::eBuffer) {

        }

        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef,
                 std::function<void(const void *src)> updateStagingFunc)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef), mUpdateStagingFunc(std::move(updateStagingFunc)),
                  vkAsset(vkAsset::Type::eBuffer) {

        }

        ~vkBuffer() {
            vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
        }

        vkBuffer(const vkBuffer &) = delete;
        vkBuffer &operator=(const vkBuffer &) = delete;

        vkBuffer(vkBuffer &&other) noexcept
                : buffer(other.buffer), vmaAllocation(other.vmaAllocation), mappedData(other.mappedData),
                  mAllocator(other.mAllocator), vkAsset(vkAsset::Type::eBuffer) {
            other.buffer = VK_NULL_HANDLE;
            other.vmaAllocation = nullptr;
            other.mappedData = nullptr;
        }

        vkBuffer &operator=(vkBuffer &&other) noexcept {
            if (this != &other) {
                this->~vkBuffer();
                buffer = other.buffer;
                vmaAllocation = other.vmaAllocation;
                mappedData = other.mappedData;
                other.buffer = VK_NULL_HANDLE;
                other.vmaAllocation = nullptr;
                other.mappedData = nullptr;
            }
            return *this;
        }

        template<typename T>
        auto updateBuf(const T &src, bool unmap = false) {
            ensureMapped();
            mUpdateStagingFunc(&src);
            if (unmap)
                unmapMem();
        }

        template<typename T>
        void updateBuf(const std::vector<T> &src, bool unmap = false) {
            ensureMapped();
            if (!src.empty())
                mUpdateStagingFunc(src.data());
            if (unmap)
                unmapMem();
        }

    private:
        auto ensureMapped() -> void {
            if (mappedData == nullptr)
                vmaMapMemory(mAllocator, vmaAllocation, &mappedData);
        }

        auto unmapMem() const -> void {
            vmaUnmapMemory(mAllocator, vmaAllocation);
        }

    private:
    };

    struct vkImage : vkAsset{
        vk::Image image;
        vk::ImageView imageView;
        vk::Sampler sampler;
        VmaAllocation vmaAllocation;
        VmaAllocator &mAllocator;

        vkImage(vk::Image img, vk::ImageView imgView, vk::Device dev, VmaAllocation alloc, VmaAllocator &allocatorRef)
                : image(img), imageView(imgView), mDevice(dev), vmaAllocation(alloc), mAllocator(allocatorRef),
                  vkAsset(vkAsset::Type::eImage) {

        }

        ~vkImage(){
            mDevice.destroy(image);
            mDevice.destroy(imageView);
            vmaFreeMemory(mAllocator, vmaAllocation);
        }

        vkImage(const vkImage &) = delete;
        vkImage &operator=(const vkImage &) = delete;

        vkImage(vkImage &&other) noexcept
        : image(other.image), imageView(other.imageView), vmaAllocation(other.vmaAllocation),
        mAllocator(other.mAllocator), vkAsset(vkAsset::Type::eImage) {
            other.image = VK_NULL_HANDLE;
            other.imageView = VK_NULL_HANDLE;
            other.vmaAllocation = nullptr;
        }

        vkImage &operator=(vkImage &&other) noexcept {
            if (this != &other) {
                this->~vkImage();
                image = other.image;
                vmaAllocation = other.vmaAllocation;
                imageView = other.imageView;
                other.image = VK_NULL_HANDLE;
                other.imageView = VK_NULL_HANDLE;
                other.vmaAllocation = nullptr;
            }
            return *this;
        }

    private:
        vk::Device mDevice;
    };

}

using vkBuf_sptr = std::shared_ptr<yic::vkBuffer>;
using vkImg_sptr = std::shared_ptr<yic::vkImage>;

#endif //VKCELSHADINGRENDERER_VKBUF_H
