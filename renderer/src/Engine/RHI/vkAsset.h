#include <utility>

//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKBUF_H
#define VKCELSHADINGRENDERER_VKBUF_H

namespace yic {

    struct vkBuffer : public Identifiable{
        vk::Buffer buffer{};
        VmaAllocation vmaAllocation{};
        void *mappedData = nullptr;
        VmaAllocator &mAllocator;
        std::function<void(const void* src)> mUpdateStagingFunc{};

        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef, const std::string& id,
                 std::function<void(const void *src)> updateStagingFunc)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef), mUpdateStagingFunc(std::move(updateStagingFunc)), Identifiable(id){

        }

        ~vkBuffer() override {
            vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
        }

        vkBuffer(const vkBuffer &) = delete;
        vkBuffer &operator=(const vkBuffer &) = delete;

        vkBuffer(vkBuffer &&other) noexcept
                : buffer(other.buffer), vmaAllocation(other.vmaAllocation), mappedData(other.mappedData),
                  mAllocator(other.mAllocator), Identifiable(other.id) {
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

    struct vkImage : public Identifiable{
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        vk::Sampler sampler{};
        std::vector<VmaAllocation> vmaAllocation{};
        VmaAllocator &mAllocator;

        size_t imageCount{0};

        vkImage(const std::vector<vk::Image>& imgs, const std::vector<vk::ImageView>& imgViews, vk::Sampler spr, vk::Device dev, const std::vector<VmaAllocation>& alloc, VmaAllocator &allocatorRef, std::string id)
                : images(imgs), imageViews(imgViews), sampler(spr), mDevice(dev), vmaAllocation(alloc), mAllocator(allocatorRef),
                  Identifiable(std::move(id)){
            imageCount = imgs.size();
        }

        ~vkImage() override{
            mDevice.destroy(sampler);
            for(auto i = imageCount; i-- > 0;){
                mDevice.destroy(images[i]);
                mDevice.destroy(imageViews[i]);
                vmaFreeMemory(mAllocator, vmaAllocation[i]);
            }
        }

        vkImage(const vkImage &) = delete;
        vkImage &operator=(const vkImage &) = delete;

        vkImage(vkImage &&other) noexcept
                : images(std::move(other.images)), imageViews(std::move(other.imageViews)), vmaAllocation(std::move(other.vmaAllocation)),
                  mAllocator(other.mAllocator), Identifiable(other.id) {
            for (auto i = imageCount; i -- > 0;){
                other.images[i] = VK_NULL_HANDLE;
                other.imageViews[i] = VK_NULL_HANDLE;
                other.vmaAllocation[i] = nullptr;
            }
        }

        vkImage &operator=(vkImage &&other) noexcept {
            if (this != &other) {
                this->~vkImage();
                images = other.images;
                vmaAllocation = other.vmaAllocation;
                imageViews = other.imageViews;
                for (auto i = imageCount; i -- > 0;){
                    other.images[i] = VK_NULL_HANDLE;
                    other.imageViews[i] = VK_NULL_HANDLE;
                    other.vmaAllocation[i] = nullptr;
                }
            }
            return *this;
        }

    private:
        vk::Device mDevice{};
    };

}

using vkBuf_sptr = std::shared_ptr<yic::vkBuffer>;
using vkImg_sptr = std::shared_ptr<yic::vkImage>;

#endif //VKCELSHADINGRENDERER_VKBUF_H
