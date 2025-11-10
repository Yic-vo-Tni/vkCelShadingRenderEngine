//
// Created by lenovo on 9/1/2025.
//

#include "CommandCollector.h"

#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/QueueFamily.h"
#include "RHI/TimelineSemaphore.h"

namespace rhi2 {

    CommandCollector::CommandCollector() {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        frames.resize(yic::systemHub.va<ev::pVkRenderContext>().frameEntries->size());
        index = yic::systemHub.va<ev::pVkRenderContext>().activeImageIndex;

        for (auto& frame : frames) {
            frame.fence = ct.device->createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));

            frame.commandPools.resize(numThread);
            for (auto& pool : frame.commandPools) {
                auto poolInfo = vk::CommandPoolCreateInfo()
                .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                .setQueueFamilyIndex(yic::qFamily->acquireQueueIndex(vot::queueType::eGraphics));
                pool.commandPool = ct.device->createCommandPool(poolInfo);
                expand(pool);
            }
        }
    }

    auto CommandCollector::bind(const CommandFn &fn) -> void {
        while (true) {
            for (auto i = 0; i < numThread; i++) {
                auto &framePool = frames[*index];
                auto &safePool = framePool.commandPools[i];

                if (safePool.mutex.try_lock()) {
                    const auto& fence = framePool.fence;
                    if (ct.device->getFenceStatus(fence) == vk::Result::eNotReady) {
                        if (ct.device->waitForFences(fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
                            throw std::runtime_error("failed to submit command buffer submission");
                        }
                    }

                    if (safePool.cmds.empty() || safePool.next > safePool.cmds.size()) {
                        expand(safePool);
                    }

                    auto &cmd = safePool.cmds[safePool.next++];
                    cmd.id = framePool.id++;

                    cmd.render([fn, &cmd] { fn(cmd); });

                    if (framePool.records.size() <= cmd.id) {
                        framePool.records.resize(cmd.id + 1);
                    }

                    framePool.records[cmd.id] = cmd;

                    safePool.mutex.unlock();
                    return;
                }
            }
        }
    }

    auto CommandCollector::bind(const std::uint32_t &order, const std::uint32_t& grow, const CommandFn &fn) -> void {
        while (true) {
            for (auto i = 0; i < numThread; i++) {
                auto &framePool = frames[*index];
                auto &safePool = framePool.commandPools[i];

                if (safePool.mutex.try_lock()) {
                    framePool.records.grow_to_at_least(grow);

                    const auto& fence = framePool.fence;
                    if (ct.device->getFenceStatus(fence) == vk::Result::eNotReady) {
                        if (ct.device->waitForFences(fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess) {
                            throw std::runtime_error("failed to submit command buffer submission");
                        }
                    }

                    if (safePool.cmds.empty() || safePool.next > safePool.cmds.size()) {
                        expand(safePool);
                    }

                    auto &cmd = safePool.cmds[safePool.next++];
                    //cmd.id = framePool.id++;
                    cmd.id = order;

                    cmd.render([fn, &cmd] { fn(cmd); });

                    // if (framePool.records.size() <= cmd.id) {
                    //     framePool.records.resize(cmd.id + 1);
                    // }

                    framePool.records[cmd.id] = cmd;

                    safePool.mutex.unlock();
                    return;
                }
            }
        }
    }

    auto CommandCollector::submit() -> void {
        auto& frame = frames[*index];
        const auto& fence = frame.fence;
        frame.id = 0;
        for (auto& pool : frame.commandPools) {
            pool.next = 0;
        }

        ct.device->resetFences(fence);

        auto cmds = vot::vector<vot::CommandBuffer>();
        cmds.reserve(frame.records.size());
        for (auto& r : frame.records)
            cmds.emplace_back(r);

        yic::timeline->submit(vot::SubmitInfo()
            .setQueueType(vot::queueType::eGraphics)
            .setWaitValues(vot::timelineStage::ePrepare)
            .setSignalValues(vot::timelineStage::eFinish)
            .setWaitStageMasks(vk::PipelineStageFlagBits::eTopOfPipe)
            .setFence(fence)
            .setCommandBuffers(cmds));
        frame.records.clear();
    }

    auto CommandCollector::clear() const -> void {
        for (auto& frame : frames) {
            ct.device->destroy(frame.fence);

            for (const auto& pool : frame.commandPools) {
                ct.device->destroy(pool.commandPool);
            }
        }
    }

    auto CommandCollector::expand(SafeCommandPool &safe_command_pool) const -> void {
        const auto allocInfo = vk::CommandBufferAllocateInfo()
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(safe_command_pool.commandPool)
        .setCommandBufferCount(numExpandCmd);

        for (auto cmds = ct.device->allocateCommandBuffers(allocInfo); auto& cmd : cmds) {
            safe_command_pool.cmds.emplace_back(cmd);
        }
    }
} // rhi2