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
        std::vector<VmaAllocation> vmaAllocation{};
        vk::Image depthImage = VK_NULL_HANDLE;
        vk::ImageView depthImageView = VK_NULL_HANDLE;
        VmaAllocation depthVmaAllocation{};
        std::vector<vk::Framebuffer> framebuffers{};
        VmaAllocator &mAllocator;

        struct info{
            size_t imageCount{0};
            size_t width{};
            size_t height{};
        } info_;

        vkImage(const std::vector<vk::Image> &imgs, const std::vector<vk::ImageView> &imgViews,
                vk::Device dev, const std::vector<VmaAllocation> &alloc,
                VmaAllocator &allocatorRef, vkImageConfig config, const std::string &id)
                : images(imgs), imageViews(imgViews),
                  mDevice(dev), vmaAllocation(alloc),
                  mAllocator(allocatorRef), Identifiable(id) {
            info_.imageCount = imgs.size();
            info_.width = config.extent.width;
            info_.height = config.extent.height;
        }
        vkImage(const std::vector<vk::Image> &imgs, const std::vector<vk::ImageView> &imgViews, const std::vector<VmaAllocation> &alloc,
                const vk::Image& depthImg, const vk::ImageView& depthImgViews, const VmaAllocation& depthAlloc,
                vk::Device dev, VmaAllocator &allocatorRef, vkImageConfig config, const std::string &id)
                : images(imgs), imageViews(imgViews), vmaAllocation(alloc),
                  depthImage(depthImg), depthImageView(depthImgViews), depthVmaAllocation(depthAlloc),
                  mDevice(dev), mAllocator(allocatorRef), Identifiable(id) {
            info_.imageCount = imgs.size();
            info_.width = config.extent.width;
            info_.height = config.extent.height;
        }
        vkImage(const std::vector<vk::Image> &imgs, const std::vector<vk::ImageView> &imgViews, const std::vector<VmaAllocation> &alloc,
                const vk::Image& depthImg, const vk::ImageView& depthImgViews, const VmaAllocation& depthAlloc,
                const std::vector<vk::Framebuffer>& framebuffers,
                vk::Device dev, VmaAllocator &allocatorRef, vkImageConfig config, const std::string &id)
                : images(imgs), imageViews(imgViews), vmaAllocation(alloc),
                  depthImage(depthImg), depthImageView(depthImgViews), depthVmaAllocation(depthAlloc),
                  framebuffers(framebuffers),
                  mDevice(dev), mAllocator(allocatorRef), Identifiable(id) {
            info_.imageCount = imgs.size();
            info_.width = config.extent.width;
            info_.height = config.extent.height;
        }

        ~vkImage() override{
            for(auto& fb : framebuffers){
                if (fb){
                    mDevice.destroy(fb);
                }
            }

            if (depthImage) {
                mDevice.destroy(depthImage);
                mDevice.destroy(depthImageView);
                vmaFreeMemory(mAllocator, depthVmaAllocation);
            }

            for(auto i = info_.imageCount; i-- > 0;){
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
            for (auto i = info_.imageCount; i -- > 0;){
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
                for (auto i = info_.imageCount; i -- > 0;){
                    other.images[i] = VK_NULL_HANDLE;
                    other.imageViews[i] = VK_NULL_HANDLE;
                    other.vmaAllocation[i] = nullptr;
                }
            }
            return *this;
        }

    protected:
        vk::Device mDevice{};
    };




}



#endif //VKCELSHADINGRENDERER_VKBUF_H
