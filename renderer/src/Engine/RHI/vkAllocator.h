//
// Created by lenovo on 6/22/2024.
//

#ifndef VKCELSHADINGRENDERER_VKALLOCATOR_H
#define VKCELSHADINGRENDERER_VKALLOCATOR_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "vkCommon.h"

namespace yic {

    class vkAllocator : public std::enable_shared_from_this<vkAllocator>{
    public:
        vkGet auto get = [](){ return Singleton<vkAllocator>::get(); };
        vkAllocator();
        ~vkAllocator();

        static auto allocBuf(VkDeviceSize deviceSize, vk::BufferUsageFlags flags, VmaMemoryUsage usage){
            return get()->allocBuf_impl(deviceSize, nullptr, (VkBufferUsageFlags)flags, usage, false);
        }
        static auto allocBuf(VkDeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags,VmaMemoryUsage usage, bool unmap = false) -> vkBuf_sptr {
            return get()->allocBuf_impl(deviceSize, data, (VkBufferUsageFlags)flags, usage, unmap);
        };
        static auto allocStagingBuf(VkDeviceSize deviceSize, const void* data, vk::BufferUsageFlags flags){
            return get()->allocBuf_staging_impl(deviceSize, data, (VkBufferUsageFlags)flags);
        }

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

        auto allocBuf_impl(VkDeviceSize deviceSize, const void* data, VkBufferUsageFlags flags, VmaMemoryUsage usage, bool unmap) -> vkBuf_sptr;
        auto allocBuf_staging_impl(VkDeviceSize deviceSize, const void* data, VkBufferUsageFlags flags) -> vkBuf_sptr;

        auto copyBuf(VkBuffer stagingBuf, VkBuffer destBuf, VkDeviceSize deviceSize) -> void;

        auto createOffScreenImage(vkImageConfig config) -> void;

        void t(){
            vk::Extent2D m;
            createOffScreenImage(vkImageConfig{m}.setArrayLayers(2));
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
