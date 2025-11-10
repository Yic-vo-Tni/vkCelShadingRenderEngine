//
// Created by lenovo on 9/25/2024.
//

#include "TimelineSemaphore.h"
#include "QueueFamily.h"
#include "Core/DispatchSystem/SystemHub.h"


namespace rhi {
    TimelineSemaphore::TimelineSemaphore() : graphicsQueue(yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics, 0)) {
        const auto typeCreateInfo = vk::SemaphoreTypeCreateInfo()
                .setSemaphoreType(vk::SemaphoreType::eTimeline)
                .setInitialValue(value);
        const auto semaphoreCreateInfo = vk::SemaphoreCreateInfo()
                .setPNext(&typeCreateInfo);

        graphicsQueue = yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics, 0);

        handle = vot::create("create timeline") = [&]{
            return yic::systemHub.va<ev::pVkSetupContext>().device->createSemaphore(semaphoreCreateInfo);
        };
    }

    TimelineSemaphore::~TimelineSemaphore() = default;

    auto TimelineSemaphore::submit(const vot::SubmitInfo& submitInfo) -> void {
        const auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(submitInfo.waitValues)
                .setSignalSemaphoreValues(submitInfo.signalValues);

        // auto sub = vk::SubmitInfo()
        //         .setCommandBuffers(submitInfo.cmds);
        // yic::logger->warn("sizeof(vk::CommandBuffer)  = {}", sizeof(vk::CommandBuffer));
        // yic::logger->warn("sizeof(vot::CommandBuffer) = {}", sizeof(vot::CommandBuffer));
        //
        // // 检查每个句柄的地址和字节内容
        // for (size_t i = 0; i < submitInfo.cmds.size(); ++i) {
        //     auto& src = submitInfo.cmds[i];
        //     yic::logger->warn(
        //         fmt::runtime("[src {}] vk handle = {}, first bytes = {:016llx}"),
        //         i,
        //         (void*)static_cast<VkCommandBuffer>(src),
        //         *reinterpret_cast<const uint64_t*>(&src)
        //     );
        // }
        //
        // // 执行转换

        // 检查转换结果
        // for (size_t i = 0; i < cmds.size(); ++i) {
        //     auto& dst = cmds[i];
        //     yic::logger->warn(
        //         fmt::runtime("[dst {}] vk handle = {}, first bytes = {:016llx}"),
        //         i,
        //         (void*)static_cast<VkCommandBuffer>(dst),
        //         *reinterpret_cast<const uint64_t*>(&dst)
        //     );
        // }
        auto cmds = vot::vector<vk::CommandBuffer>(submitInfo.cmds.begin(), submitInfo.cmds.end());
        auto sub = vk::SubmitInfo()
            .setCommandBuffers(cmds);

        if (!submitInfo.onetimeSubmit) {
            sub.setWaitDstStageMask(submitInfo.waitStageMasks)
                    .setSignalSemaphores(handle)
                    .setPNext(&timelineSubmitInfo);
            if (!submitInfo.waitValues.empty())
                sub.setWaitSemaphores(handle);
        }

        if (submitInfo.queue == vot::queueType::eUndefined){
            graphicsQueue.submit(sub, submitInfo.fence ? submitInfo.fence : submitInfo.cmds.front().fence);
        } else {
            auto q = yic::qFamily->acquireQueue(submitInfo.queue, submitInfo.selectQueue);
            q->submit(sub, submitInfo.fence ? submitInfo.fence : submitInfo.cmds.front().fence);
        }
    }

    auto TimelineSemaphore::finalSubmit(vk::SwapchainKHR swapchainKhr, uint32_t imageIndex, const vot::SubmitInfo &submitInfo) -> vk::Result {
        const auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(submitInfo.waitValues)
                .setSignalSemaphoreValues(submitInfo.signalValues);

        const vk::Semaphore wait[] = {handle, submitInfo.waitSemaphore};
        const vk::Semaphore signal[] = {handle, submitInfo.signalSemaphore};

        const auto sub = vk::SubmitInfo()
                .setCommandBuffers(submitInfo.cmds)
                .setWaitSemaphores(wait)
                .setWaitDstStageMask(submitInfo.waitStageMasks)
                .setSignalSemaphores(signal)
                .setPNext(&timelineSubmitInfo);

        vot::SubmitInfo::increase();

        graphicsQueue.submit(sub, submitInfo.cmds.front().fence);

        vk::Result r;
        try {
            r = graphicsQueue.presentKHR(vk::PresentInfoKHR()
                .setSwapchains(swapchainKhr)
                .setImageIndices(imageIndex)
                .setWaitSemaphores(submitInfo.signalSemaphore));
        } catch (vk::OutOfDateKHRError&) {
            return vk::Result::eErrorOutOfDateKHR;
        }
        return r;
    }

    auto TimelineSemaphore::clear() const -> void {
        yic::systemHub.va<ev::pVkSetupContext>().device->destroy(handle);
    }
} // rhi
