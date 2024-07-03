#include <utility>

//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKBUF_H
#define VKCELSHADINGRENDERER_VKBUF_H

namespace un {

    struct vkBuffer {
        vk::Buffer buffer;
        VmaAllocation vmaAllocation;
        void *mappedData = nullptr;
        VmaAllocator &mAllocator;
        std::function<void()> mUpdateStagingFunc;

        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef) {

        }
        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef, std::function<void()> updateStagingFunc)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef), mUpdateStagingFunc(std::move(updateStagingFunc)) {

        }

        ~vkBuffer() {
//            if (mappedData) {
//                vmaUnmapMemory(mAllocator, vmaAllocation);
//            }
            vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
            //vmaFreeMemory(mAllocator, vmaAllocation);
        }

        vkBuffer(const vkBuffer &) = delete;

        vkBuffer &operator=(const vkBuffer &) = delete;

        vkBuffer(vkBuffer &&other) noexcept
                : buffer(other.buffer), vmaAllocation(other.vmaAllocation), mappedData(other.mappedData),
                  mAllocator(other.mAllocator) {
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
            memcpy(mappedData, &src, sizeof(src));
            if (unmap)
                unmapMem();
        }

        template<typename T>
        void updateBuf(const std::vector<T> &src, bool unmap = false) {
            ensureMapped();
            if (!src.empty())
                memcpy(mappedData, src.data(), src.size() * sizeof(T));
            if (unmap)
                unmapMem();
        }

        template<typename T>
        auto updateBuf(const T &src, size_t size, bool unmap = false) {
            ensureMapped();
            memcpy(mappedData, &src, size);
            if (unmap)
                unmapMem();
        }

        template<typename T>
        auto updateBuf(const T *src, size_t size, size_t offset, bool unmap = false) {
            ensureMapped();
            memcpy(static_cast<const uint8_t *>(mappedData) + offset, src, size);
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

    struct vkImage{
        vk::Image image;
        vk::ImageView imageView;
        vk::Sampler sampler;
        VmaAllocation vmaAllocation;
        VmaAllocator &mAllocator;

        vkImage(vk::Image img, VmaAllocation alloc, VmaAllocator &allocatorRef) : image(img), vmaAllocation(alloc), mAllocator(allocatorRef){

        }
    };

}

using vkBuf_sptr = std::shared_ptr<un::vkBuffer>;

#endif //VKCELSHADINGRENDERER_VKBUF_H
