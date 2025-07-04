//
// Created by lenovo on 9/25/2024.
//

#include "Command.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "QueueFamily.h"
#include "TimelineSemaphore.h"

namespace rhi {
    CommandManager::CommandManager() : ct(yic::systemHub.val<ev::pVkSetupContext>()),
                                       mActiveImageIndex(yic::systemHub.val<ev::pVkRenderContext>().activeImageIndex) {
        for(auto i = 0; i < mMaxPoolCount; i++){
            allocCommandBuffer();
        }

        mFrameCount = (uint32_t )yic::systemHub.val<ev::pVkRenderContext>().frameEntries->size();
        mThreadCommandPools.resize(vot::threadSpecificCmdPool::eCount);
        mThreadCommandbuffers.resize(vot::threadSpecificCmdPool::eCount);
        for(auto i = 0; i < vot::threadSpecificCmdPool::eCount; i++){
            switch (static_cast<vot::threadSpecificCmdPool>(i)) {
                case vot::threadSpecificCmdPool::eMainRender: {
                    vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, yic::qFamily->acquireQueueIndex(vot::queueType::eGraphics)};
                    auto pool = ct.device->createCommandPool(poolInfo);
                    mThreadCommandPools[vot::threadSpecificCmdPool::eMainRender] = pool;

                    break;
                }
                case vot::threadSpecificCmdPool::eSceMain: {
                    vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, yic::qFamily->acquireQueueIndex(vot::queueType::eGraphics)};
                    auto pool = ct.device->createCommandPool(poolInfo);
                    mThreadCommandPools[vot::threadSpecificCmdPool::eSceMain] = pool;

                    break;
                }
                default:
                    break;
            }
        }
    }

    CommandManager::~CommandManager() = default;

    auto CommandManager::acquire(const vot::threadSpecificCmdPool &threadSpecificCmdPool) -> vot::CommandBuffer* {
        auto index = mFrameCount * mIndexManager.allocate();

        if (index >= mThreadCommandbuffers[threadSpecificCmdPool].size())
            alloc(threadSpecificCmdPool);
        auto& cmd = mThreadCommandbuffers[threadSpecificCmdPool][mFrameCount * index];

        return &cmd;
    }

    auto CommandManager::bind(vot::CommandBuffer *cmd, const std::function<void(vot::CommandBuffer &)> &fn) -> void {
        auto actualCmd = cmd + *mActiveImageIndex;

        if (ct.device->getFenceStatus(actualCmd->fence) == vk::Result::eNotReady) {
            if (ct.device->waitForFences(actualCmd->fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
                throw std::runtime_error("failed to wait fence\n");
        }
        ct.device->resetFences(actualCmd->fence);

        fn(*actualCmd);
    }

    auto CommandManager::bind(vot::SubmitInfo submitInfo,
                              const std::function<void(vot::CommandBuffer &)> &fn) -> void {
        auto actualCmd = static_cast<vot::CommandBuffer*>(submitInfo.pNext) + *mActiveImageIndex;

        if (ct.device->getFenceStatus(actualCmd->fence) == vk::Result::eNotReady) {
            if (ct.device->waitForFences(actualCmd->fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
                throw std::runtime_error("failed to wait fence\n");
        }
        ct.device->resetFences(actualCmd->fence);

        actualCmd->render([&]{fn(*actualCmd);});
        //fn(*actualCmd);

        yic::timeline->submit(submitInfo.setCommandBuffers(*actualCmd));
    }

    ///---------------------------------------------------------

    auto CommandManager::acquire() -> vot::CommandBuffer {
        vot::CommandBuffer cmd;

        if (mAvailablePrimaryCommandbuffers.try_pop(cmd)){

            return cmd;
        }

        std::unique_lock<std::mutex> lock(mMutex);
        mCommandAvailable.wait(lock, [this] { return !mAvailablePrimaryCommandbuffers.empty();});

        mAvailablePrimaryCommandbuffers.try_pop(cmd);

        return cmd;
    }

    auto CommandManager::release(vot::CommandBuffer &cmd) -> void {
        if (ct.device->waitForFences(cmd.fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess){

        }
        cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        ct.device->resetFences(cmd.fence);

        mAvailablePrimaryCommandbuffers.push(cmd);

        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCommandAvailable.notify_one();
        }
    }

    auto CommandManager::draw(const std::function<void(vot::CommandBuffer &)> &record) -> void {
        auto cmd = acquire();
        record(cmd);
        release(cmd);
    }

    auto CommandManager::drawOneTimeSubmit(const std::function<void(vot::CommandBuffer &)> &rec) -> void {
        auto cmd = acquire();

        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        rec(cmd);
        cmd.end();

        yic::timeline->submit(vot::SubmitInfo()
                                      .setQueueType(vot::queueType::eGraphics)
                                      .setSelectQueue(1)
                                      .setCommandBuffers(cmd)
                                      .useOnetimeSubmit());
        release(cmd);
    }

    auto CommandManager::clear() -> void {
        for(auto& fence : mFences){
            ct.device->destroy(fence);
        }

        for(auto& pool : mCommandPools){
            ct.device->destroy(pool);
        }

        for(auto& cmds : mThreadCommandbuffers){
            for(auto& cmd : cmds){
                ct.device->destroy(cmd.fence);
            }
        }

        for(auto& pool : mThreadCommandPools){
            ct.device->destroy(pool);
        }
    }

    auto CommandManager::allocCommandBuffer() -> void {
        vk::CommandPoolCreateInfo poolInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, yic::qFamily->acquireQueueIndex(vot::queueType::eGraphics)};
        auto pool = ct.device->createCommandPool(poolInfo);
        mCommandPools.emplace_back(pool);

        vk::CommandBufferAllocateInfo allocInfo{ pool, vk::CommandBufferLevel::ePrimary, 1 };
        auto primary = ct.device->allocateCommandBuffers(allocInfo).front();

        auto fence = ct.device->createFence(vk::FenceCreateInfo());
        mFences.emplace_back(fence);

        mAvailablePrimaryCommandbuffers.push({primary, fence});
    }

    auto CommandManager::alloc(const vot::threadSpecificCmdPool& threadSpecificCmdPool) -> void {
        vk::CommandBufferAllocateInfo allocInfo{mThreadCommandPools[threadSpecificCmdPool], vk::CommandBufferLevel::ePrimary, mFrameCount};
        auto cmds = ct.device->allocateCommandBuffers(allocInfo);
        auto oldSize = mThreadCommandbuffers[threadSpecificCmdPool].size();
        for(auto& cmd : cmds){
            mThreadCommandbuffers[threadSpecificCmdPool].emplace_back(cmd);
        }
        for(auto i = oldSize; i < mThreadCommandbuffers[threadSpecificCmdPool].size(); ++i){
            mThreadCommandbuffers[threadSpecificCmdPool][i].fence = ct.device->createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
        }
    }
} // rhi