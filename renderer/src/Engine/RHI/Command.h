//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMAND_H
#define VKCELSHADINGRENDERER_VKCOMMAND_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class FenceManager{
    public:
        explicit FenceManager(vk::Device device) : mDevice(device) {
            for (int i = 0; i < 4; i++) {
                mAvailableFences.push(mDevice.createFence(vk::FenceCreateInfo()));
                mCurrentFenceCount += 1;
            }
        }

        ~FenceManager(){
            vk::Fence f;
            while(mAvailableFences.try_pop(f)){
                mDevice.destroy(f);
            }
        }

        auto acquire() -> vk::Fence{
            vk::Fence fence;

            if (mAvailableFences.try_pop(fence)){
                return fence;
            }

            if (mCurrentFenceCount < mMaxFenceCount){
                mCurrentFenceCount += 1;
                fence = mDevice.createFence(vk::FenceCreateInfo());
                return fence;
            }

            std::unique_lock<std::mutex> lock(mLock);
            mFenceAvailable.wait(lock, [this]{ return !mAvailableFences.empty();});
            mAvailableFences.try_pop(fence);

            return fence;
        };

        auto release(vk::Fence fence) -> void{
            mDevice.resetFences(fence);
            mAvailableFences.push(fence);
            {
                std::lock_guard<std::mutex> lock(mLock);
                mFenceAvailable.notify_one();
            }
        }

    private:
        static constexpr uint32_t mMaxFenceCount{12};
        uint32_t mCurrentFenceCount{0};
        vk::Device mDevice;
        std::mutex mLock;
        std::condition_variable mFenceAvailable;
        oneapi::tbb::concurrent_queue<vk::Fence> mAvailableFences;
    };

    class CommandPoolManager{
    public:
        CommandPoolManager(vk::Device device, uint32_t queueFamilyIndex) : mDevice(device), mQueueFamilyIndex(queueFamilyIndex) {
            for (int i = 0; i < 4; i++) {
                vk::CommandPoolCreateInfo poolInfo = {};
                poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
                poolInfo.setQueueFamilyIndex(queueFamilyIndex);
                auto pool = mDevice.createCommandPool(poolInfo);
                mCommandPools.emplace_back(pool);

                vk::CommandBufferAllocateInfo allocInfo{
                        pool, vk::CommandBufferLevel::ePrimary, 1
                };
                mAvailableCommandBufs.push(mDevice.allocateCommandBuffers(allocInfo).front());
                mCurrentPoolCount += 1;
            }
        }

        ~CommandPoolManager(){
            for(auto& pool : mCommandPools){
                mDevice.destroy(pool);
            }
        }

        auto acquire() -> vk::CommandBuffer{
            vk::CommandBuffer cmd;

            if (mAvailableCommandBufs.try_pop(cmd)){
                return cmd;
            }

            if (mCurrentPoolCount < mMaxPoolCount){
                mCurrentPoolCount += 1;
                vk::CommandPoolCreateInfo poolInfo = {};
                poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
                poolInfo.setQueueFamilyIndex(mQueueFamilyIndex);
                auto pool = mDevice.createCommandPool(poolInfo);
                mCommandPools.emplace_back(pool);
                vk::CommandBufferAllocateInfo allocInfo{
                        pool, vk::CommandBufferLevel::ePrimary, 1
                };
                cmd = mDevice.allocateCommandBuffers(allocInfo).front();
                return cmd;
            }

            std::unique_lock<std::mutex> lock(mLock);
            mCommandBufAvailable.wait(lock, [this]{ return !mAvailableCommandBufs.empty();});
            mAvailableCommandBufs.try_pop(cmd);

            return cmd;
        };

        auto release(vk::CommandBuffer cmd) -> void{
            cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
            mAvailableCommandBufs.push(cmd);
            {
                std::lock_guard<std::mutex> lock(mLock);
                mCommandBufAvailable.notify_one();
            }
        }

    private:
        static constexpr uint32_t mMaxPoolCount{12};
        uint32_t mCurrentPoolCount{0};
        vk::Device mDevice;
        uint32_t mQueueFamilyIndex;
        std::mutex mLock;
        std::condition_variable mCommandBufAvailable;
        std::vector<vk::CommandPool> mCommandPools;
        oneapi::tbb::concurrent_queue<vk::CommandBuffer> mAvailableCommandBufs;
    };

    class CommandBufferCoordinator{
    public:
        CommandBufferCoordinator(vk::Device device, uint32_t queueFamilyIndex, vk::Queue queue);
        ~CommandBufferCoordinator();

        auto cmdAcquire() -> vk::CommandBuffer;
        auto cmdRelease(vk::CommandBuffer& cmd) -> void;
        auto cmdDraw(const std::function<void(vk::CommandBuffer&)>& fn) -> void;

    private:
        vk::Device mDevice;
        vk::Queue mQueue;
        oneapi::tbb::spin_mutex mMutex;
        std::unique_ptr<CommandPoolManager> mCommandPoolManager;
        std::unique_ptr<FenceManager> mFenceManager;
        oneapi::tbb::concurrent_queue<vk::CommandBuffer> mAvailableCommandBuffers;
    };

} // yic

#endif //VKCELSHADINGRENDERER_COMMAND_H
