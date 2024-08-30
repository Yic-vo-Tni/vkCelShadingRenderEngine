//
// Created by lenovo on 6/10/2024.
//

#include "Command.h"

namespace yic {

    CommandBufferCoordinator::CommandBufferCoordinator() = default;

    CommandBufferCoordinator::~CommandBufferCoordinator() = default;

    auto CommandBufferCoordinator::cmdAcquirePrimary_impl() -> vk::CommandBuffer{
        auto cmd = mCommandPoolManager->acquire();

        cmd.begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        return cmd;
    }

    auto CommandBufferCoordinator::cmdReleasePrimary_impl(vk::CommandBuffer& cmd) -> void {
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

        cmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        mCommandPoolManager->release(cmd);
        mFenceManager->release(fence);
    }

    auto CommandBufferCoordinator::cmdDrawPrimary_impl(const std::function<void(vk::CommandBuffer &)> &fn) -> void {
        auto cmd = cmdAcquirePrimary_impl();
        fn(cmd);
        cmdReleasePrimary_impl(cmd);
    }

    auto CommandBufferCoordinator::cmdDrawSecond_impl(vk::RenderPass rp, vk::Extent2D extent, const std::function<void(vk::CommandBuffer&)>& fn) -> vk::CommandBuffer {
        auto entry = mInheritanceManager->acquire();
        auto queue = entry->commandBuffer;
        auto cmd = queue.front();
        queue.pop();

        vk::Viewport viewport{
                0.f, 0.f,
                static_cast<float>(extent.width), static_cast<float>(extent.height),
                0.f, 1.f
        };
        vk::Rect2D scissor{{0, 0}, extent};

        vk::CommandBufferInheritanceInfo inheritanceInfo{
                rp, 0
        };
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse, &inheritanceInfo};

        cmd.begin(beginInfo);
        cmd.setScissor(0, scissor);
        cmd.setViewport(0, viewport);
        fn(cmd);
        cmd.end();

        //mInheritanceManager->release(entry);

        return cmd;
    }


    auto CommandBufferCoordinator::init(vk::Device device, uint32_t queueFamilyIndex, vk::Queue queue) -> void {
        get()->mCommandPoolManager = std::make_unique<CommandPoolManager>(device, queueFamilyIndex);
        get()->mInheritanceManager = std::make_unique<CommandPoolInheritanceManager>(device, queueFamilyIndex);
        get()->mFenceManager = std::make_unique<FenceManager>(device);
        get()->mDevice = device;
        get()->mQueue = queue;
    }

    auto CommandBufferCoordinator::clear() -> void {
        get()->mCommandPoolManager.reset();
        get()->mInheritanceManager.reset();
        get()->mFenceManager.reset();
    }

    auto CommandBufferCoordinator::begin() -> void {

    }




























































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