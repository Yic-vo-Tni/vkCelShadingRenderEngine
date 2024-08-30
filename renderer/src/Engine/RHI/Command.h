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
                createCommandBuf();
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

                if (mAvailablePrimaryCommandBufs.try_pop(cmd)) {
                    return cmd;
                }

                if (mCurrentPoolCount < mMaxPoolCount) {
                    mCurrentPoolCount += 1;

                    createCommandBuf();
                    mAvailablePrimaryCommandBufs.try_pop(cmd);
                    return cmd;
                }

                std::unique_lock<std::mutex> lock(mLock);
                mCommandBufAvailable.wait(lock, [this] { return !mAvailablePrimaryCommandBufs.empty(); });

            return cmd;
        }

        auto release(const vk::CommandBuffer& cmd) -> void{
                mAvailablePrimaryCommandBufs.push(cmd);
            {
                std::lock_guard<std::mutex> lock(mLock);
                mCommandBufAvailable.notify_one();
            }
        }

    private:
        auto createCommandBuf() -> void{
            vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mQueueFamilyIndex};
            auto pool = mDevice.createCommandPool(poolInfo);
            mCommandPools.emplace_back(pool);

            vk::CommandBufferAllocateInfo allocInfo{
                    pool, vk::CommandBufferLevel::ePrimary, 1
            };
            auto primary = mDevice.allocateCommandBuffers(allocInfo).front();

            mAvailablePrimaryCommandBufs.push(primary);
        };



    private:
        static constexpr uint32_t mMaxPoolCount{12};
        uint32_t mCurrentPoolCount{0};
        vk::Device mDevice;
        uint32_t mQueueFamilyIndex;
        std::mutex mLock;
        std::condition_variable mCommandBufAvailable;
        std::vector<vk::CommandPool> mCommandPools;
        oneapi::tbb::concurrent_queue<vk::CommandBuffer> mAvailablePrimaryCommandBufs;
    };

    class CommandPoolInheritanceManager{
        struct Entry{
            bool use;
            vk::CommandPool commandPool;
            std::queue<vk::CommandBuffer> commandBuffer;
        };
    public:
        CommandPoolInheritanceManager(vk::Device device, uint32_t queueFamilyIndex) : mDevice(device), mQueueFamilyIndex(queueFamilyIndex) {
            for (int i = 0; i < 6; i++) {
                createCommandPool();
            }
        }

        ~CommandPoolInheritanceManager(){
            for(auto& e : mEntries){
                mDevice.destroy(e.commandPool);
            }
        }

        auto acquire() -> Entry*{
            auto it = std::ranges::find_if(mEntries, [](const Entry& e){
                return !e.use;
            });
            if (it != mEntries.end()){
                if (it->commandBuffer.empty()){
                    it->commandBuffer.push(createCommandBuf(it->commandPool));
                }
            }
            it->use = true;

            return &(*it);
        }

        auto release(Entry* entry) -> void{
            entry->use = false;
        }

    private:
        auto createCommandPool() -> void{
            vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, mQueueFamilyIndex};
            auto pool = mDevice.createCommandPool(poolInfo);
            mEntries.emplace_back(Entry{false, pool});
        }
        auto createCommandBuf(const vk::CommandPool& pool) -> vk::CommandBuffer{
            vk::CommandBufferAllocateInfo allocInfo{
                    pool, vk::CommandBufferLevel::eSecondary, 1
            };

            auto second = mDevice.allocateCommandBuffers(allocInfo).front();
            return second;
        };

    private:
        vk::Device mDevice;
        uint32_t mQueueFamilyIndex;
        std::vector<Entry> mEntries;
    };

    class CommandBufferCoordinator{
    public:
        vkGet auto get = []{ return Singleton<CommandBufferCoordinator>::get();};

        CommandBufferCoordinator();
        ~CommandBufferCoordinator();

        static auto init(vk::Device device, uint32_t queueFamilyIndex, vk::Queue queue) -> void;
        static auto clear() -> void;

        DEFINE_STATIC_ACCESSOR(cmdAcquirePrimary);
        DEFINE_STATIC_ACCESSOR_PARAM(cmdReleasePrimary, (vk::CommandBuffer& cmd), (cmd));
        DEFINE_STATIC_ACCESSOR_PARAM(cmdDrawPrimary, (const std::function<void(vk::CommandBuffer&)>& fn), (fn));
        DEFINE_STATIC_ACCESSOR_PARAM(cmdDrawSecond, (vk::RenderPass rp, vk::Extent2D extent, const std::function<void(vk::CommandBuffer&)>& fn), (rp, extent, fn));


#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
        static void buildAccelerationStructuresKHR(
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::AccelerationStructureBuildGeometryInfoKHR> const &      infos,
                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::AccelerationStructureBuildRangeInfoKHR * const> const & pBuildRangeInfos
                )  VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS{
            cmdDrawPrimary([&](vk::CommandBuffer& cmd){
               cmd.buildAccelerationStructuresKHR(infos, pBuildRangeInfos, EventBus::Get::vkSetupContext().dynamicDispatcher_ref());
            });
        };
#endif /* VULKAN_HPP_DISABLE_ENHANCED_MODE */

    private:
        auto cmdAcquirePrimary_impl() -> vk::CommandBuffer;
        auto cmdReleasePrimary_impl(vk::CommandBuffer& entry) -> void;
        auto cmdDrawPrimary_impl(const std::function<void(vk::CommandBuffer&)>& fn) -> void;

        auto cmdDrawSecond_impl(vk::RenderPass rp, vk::Extent2D extent, const std::function<void(vk::CommandBuffer&)>& fn) -> vk::CommandBuffer;


        /// t
        auto begin() -> void;
    private:
        vk::Device mDevice;
        vk::Queue mQueue;
        oneapi::tbb::spin_mutex mMutex;
        std::unique_ptr<CommandPoolManager> mCommandPoolManager;
        std::unique_ptr<FenceManager> mFenceManager;

        std::unique_ptr<CommandPoolInheritanceManager> mInheritanceManager;
        oneapi::tbb::concurrent_unordered_map<std::string, vk::Extent2D> mExtents;
    };

} // yic

#endif //VKCELSHADINGRENDERER_COMMAND_H
