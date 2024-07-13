#include <utility>

//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKBUF_H
#define VKCELSHADINGRENDERER_VKBUF_H

namespace yic {

//    class vkAsset : public Identifiable{
//    public:
//        enum class Type{
//            eBuffer, eImage
//        };
//    public:
//        vkAsset(std::string id, Type type) : type(type), Identifiable(std::move(id)){};
//        ~vkAsset() override = default;
//
//        Type type;
//    };

    struct vkBuffer : public Identifiable{
        vk::Buffer buffer{};
        VmaAllocation vmaAllocation{};
        void *mappedData = nullptr;
        VmaAllocator &mAllocator;
        std::function<void(const void* src)> mUpdateStagingFunc{};

//        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef)
//                : buffer(buf),
//                  vmaAllocation(alloc),
//                  mappedData(data),
//                  mAllocator(allocatorRef){
//
//        }

        vkBuffer(vk::Buffer buf, VmaAllocation alloc, void *data, VmaAllocator &allocatorRef, std::string id,
                 std::function<void(const void *src)> updateStagingFunc)
                : buffer(buf),
                  vmaAllocation(alloc),
                  mappedData(data),
                  mAllocator(allocatorRef), mUpdateStagingFunc(std::move(updateStagingFunc)), Identifiable(std::move(id)){

        }

        ~vkBuffer() override {
            vmaDestroyBuffer(mAllocator, buffer, vmaAllocation);
        }

        vkBuffer(const vkBuffer &) = delete;
        vkBuffer &operator=(const vkBuffer &) = delete;

//        vkBuffer(vkBuffer &&other) noexcept
//                : buffer(other.buffer), vmaAllocation(other.vmaAllocation), mappedData(other.mappedData),
//                  mAllocator(other.mAllocator) {
//            other.buffer = VK_NULL_HANDLE;
//            other.vmaAllocation = nullptr;
//            other.mappedData = nullptr;
//        }

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
        vk::Image image{};
        vk::ImageView imageView{};
        vk::Sampler sampler{};
        VmaAllocation vmaAllocation{};
        VmaAllocator &mAllocator;

//        vkImage(vk::Image img, vk::ImageView imgView, vk::Sampler spr, vk::Device dev, VmaAllocation alloc, VmaAllocator &allocatorRef)
//                : image(img), imageView(imgView), sampler(spr), mDevice(dev), vmaAllocation(alloc), mAllocator(allocatorRef){
//
//        }
        vkImage(vk::Image img, vk::ImageView imgView, vk::Sampler spr, vk::Device dev, VmaAllocation alloc, VmaAllocator &allocatorRef, std::string id)
                : image(img), imageView(imgView), sampler(spr), mDevice(dev), vmaAllocation(alloc), mAllocator(allocatorRef),
                  Identifiable(std::move(id)){

        }

        ~vkImage() override{
            mDevice.destroy(sampler);
            mDevice.destroy(image);
            mDevice.destroy(imageView);
            vmaFreeMemory(mAllocator, vmaAllocation);
        }

        vkImage(const vkImage &) = delete;
        vkImage &operator=(const vkImage &) = delete;

//        vkImage(vkImage &&other) noexcept
//        : image(other.image), imageView(other.imageView), vmaAllocation(other.vmaAllocation),
//        mAllocator(other.mAllocator) {
//            other.image = VK_NULL_HANDLE;
//            other.imageView = VK_NULL_HANDLE;
//            other.vmaAllocation = nullptr;
//        }

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
        vk::Device mDevice{};
    };

}

using vkBuf_sptr = std::shared_ptr<yic::vkBuffer>;
using vkImg_sptr = std::shared_ptr<yic::vkImage>;

#endif //VKCELSHADINGRENDERER_VKBUF_H
