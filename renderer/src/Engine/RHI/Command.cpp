//
// Created by lenovo on 6/10/2024.
//

#include "Command.h"

namespace yic {

    CommandBufferCoordinator::CommandBufferCoordinator(vk::Device device, uint32_t queueFamilyIndex, vk::Queue queue): mDevice(device), mQueue(queue) {
        mCommandPoolManager = std::make_unique<CommandPoolManager>(device, queueFamilyIndex);
        mFenceManager = std::make_unique<FenceManager>(device);
    }

    CommandBufferCoordinator::~CommandBufferCoordinator() {
        mCommandPoolManager.reset();
        mFenceManager.reset();
    }

    auto CommandBufferCoordinator::cmdAcquire() -> vk::CommandBuffer {
        auto cmd = mCommandPoolManager->acquire();

        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        return cmd;
    }

    auto CommandBufferCoordinator::cmdRelease(vk::CommandBuffer &cmd) -> void {
        cmd.end();
        auto fence = mFenceManager->acquire();

        vk::SubmitInfo info{{}, {}, cmd, {}};
        {
            oneapi::tbb::spin_mutex::scoped_lock lock(mMutex);
            mQueue.submit(info, fence);
        }

        if (mDevice.waitForFences({fence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess){
            throw std::runtime_error("cmd buf coordinator Failed to wait for the fence!");
        }

        mCommandPoolManager->release(cmd);
        mFenceManager->release(fence);
    }

    auto CommandBufferCoordinator::cmdDraw(const std::function<void(vk::CommandBuffer &)> &fn) -> void {
        auto cmd = cmdAcquire();
        fn(cmd);
        cmdRelease(cmd);
    };


























































//    auto CommandBufferCoordinator::cmdAcquire() -> vk::CommandBuffer {
//        vk::CommandBuffer cmdBuffer;
//        if (!mAvailableCommandBuffers.try_pop(cmdBuffer)) {
//            vk::CommandBufferAllocateInfo allocInfo = {};
//            allocInfo.setCommandPool(mCommandPool);
//            allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
//            allocInfo.setCommandBufferCount(1);
//            auto cmdBuffers = mDevice.allocateCommandBuffers(allocInfo);
//            cmdBuffer = cmdBuffers.front();
//            auto beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
//            cmdBuffer.begin(beginInfo);
//        }
//
//        return cmdBuffer;
//    }
//
//    auto CommandBufferCoordinator::cmdRelease(vk::CommandBuffer cmdBuffer) -> void {
//
//        mAvailableCommandBuffers.push(cmdBuffer);
//    }
//
//    auto CommandBufferCoordinator::cmdDraw(const std::function<void(vk::CommandBuffer&)>& fn) -> void {
//        auto cmd = cmdAcquire();
//        fn(cmd);
//        cmdRelease(cmd);
//    }
//
//    auto CommandBufferCoordinator::cmdsSubmitWhole() -> void {
//        if (mAvailableCommandBuffers.empty())
//            return;
//
//        std::vector<vk::CommandBuffer> cmdToSubmit;
//        vk::CommandBuffer tempCmd;
//        while(mAvailableCommandBuffers.try_pop(tempCmd)){
//            tempCmd.end();
//            cmdToSubmit.emplace_back(tempCmd);
//        }
//
//        vk::SubmitInfo submitInfo{{}, {}, cmdToSubmit};
//        mQueue.submit({submitInfo}, mFence);
//
//        if (mDevice.waitForFences({mFence}, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess){
//            throw std::runtime_error("cmd buf coordinator Failed to wait for the fence!");
//        }
//        mDevice.resetFences(mFence);
//
//        for(auto& cmd : cmdToSubmit){
//            cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
//            auto beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
//            cmd.begin(beginInfo);
//            mAvailableCommandBuffers.push(cmd);
//        }
//    }

} // yic