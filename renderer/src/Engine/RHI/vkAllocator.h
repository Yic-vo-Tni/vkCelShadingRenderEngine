//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKALLOCATOR_H
#define VKCELSHADINGRENDERER_VKALLOCATOR_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"
#include "Engine/RHI/vkAsset.h"

#include "vkCommon.h"

#define DEFINE_STATIC_ACCESSOR(func, param, types) \
    static auto func param { \
        return get()->func##_impl types; \
    }
#define DEFINE_STATIC_RETYPE_ACCESSOR(func, ret_type, param, types) \
    static ret_type func param { \
        return get()->func##_impl types; \
    }

namespace yic {

    class vkAllocator : public std::enable_shared_from_this<vkAllocator>{
        using imgPath = std::variant<std::string, std::vector<std::string>>;
    public:
        enum class MemoryUsage{
            eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
            eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
            eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
            eGpuToCPU = VMA_MEMORY_USAGE_GPU_TO_CPU,
        };

        enum class AllocStrategy{
            eDefault = 0,
            eMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
            eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
            eHostSequentialWrite = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        };
    private:
        struct bufCreateInfo{
            vk::DeviceSize deviceSize;
            vk::BufferUsageFlags usage;

            MemoryUsage memoryUsage;
            AllocStrategy allocStrategy;
        };

        struct buf{
            VkBuffer buf;
            VmaAllocation vmaAllocation;
        };
        struct img{
            VkImage img{};
            VmaAllocation vmaAllocation{};
            vk::ImageView imgView;
        };
    public:
        vkGet auto get = [](){ return Singleton<vkAllocator>::get(); };
        vkAllocator();
        ~vkAllocator();

        DEFINE_STATIC_ACCESSOR(allocBuf,
                               (vk::DeviceSize ds, vk::BufferUsageFlags fg, MemoryUsage us),
                               (ds, nullptr, fg, us, false));

        DEFINE_STATIC_ACCESSOR(allocBuf,
                    (vk::DeviceSize deviceSize, const void *data, vk::BufferUsageFlags flags, MemoryUsage usage, bool unmap = false),
                    (deviceSize, data, flags, usage, unmap));

        DEFINE_STATIC_ACCESSOR(allocBufStaging,
                               (vk::DeviceSize ds, const void* data, vk::BufferUsageFlags fg, MemoryUsage us),
                               (ds, data, fg, us));

        DEFINE_STATIC_ACCESSOR(allocImg,
                               (const imgPath& pt),
                               (pt));

        DEFINE_STATIC_ACCESSOR(allocImg,
                               (const imgPath& pt, vkImageConfig cf),
                               (pt, cf));

        DEFINE_STATIC_ACCESSOR(allocImgOffScreen,
                               (const vkImageConfig& cf),
                               (cf));

    private:
        vkAllocator& createTempCmdBuf(){
            vk::CommandBufferAllocateInfo allocateInfo{mTransferPool, vk::CommandBufferLevel::ePrimary, 1};
            mTempCmd = mDevice.allocateCommandBuffers(allocateInfo).front();

            vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
            mTempCmd.begin(beginInfo);

            return *this;
        }
        vkAllocator& submitTempCmdBuf(){
            mTempCmd.end();

            vk::SubmitInfo submitInfo{};
            submitInfo.setCommandBuffers(mTempCmd);
            mGraphicsQueue.submit(submitInfo);
            mGraphicsQueue.waitIdle();
            mDevice.free(mTransferPool, mTempCmd);

            return *this;
        }

        auto allocBuf_impl(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags, MemoryUsage usage, bool unmap) -> vkBuf_sptr;
        auto allocBufStaging_impl(vk::DeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags,
                                   MemoryUsage usage = MemoryUsage::eGpuOnly,  AllocStrategy allocStrategy = AllocStrategy::eDefault) -> vkBuf_sptr;
        auto createBuf(const bufCreateInfo& bufCreateInfo) -> buf;
        auto copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize) -> void;
        auto mapBuf(const buf& buf, VkDeviceSize devSize, const void* data, bool unmap) -> void*;

        auto allocImg_impl(const imgPath& imgPath, std::optional<vkImageConfig> config = std::nullopt) -> vkImg_sptr;
        auto allocImgOffScreen_impl(vkImageConfig config) -> vkImg_sptr;
        auto copyBufToImg(VkBuffer buf, VkImage img, uint32_t w, uint32_t h) -> void;
        auto transitionImageLayout(VkImage image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) -> void;

        void t(){
            vk::Extent2D m;
            allocImgOffScreen_impl(vkImageConfig{m}.setArrayLayers(2));

            allocImg("slfdj");
        }
    private:
        vk::Device mDevice;
        vk::Queue mGraphicsQueue;

        vk::CommandPool mTransferPool{};
        vk::CommandBuffer mTempCmd{};
        VmaAllocator mVmaAllocator{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKALLOCATOR_H
