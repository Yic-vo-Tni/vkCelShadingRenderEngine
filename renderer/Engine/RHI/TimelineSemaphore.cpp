//
// Created by lenovo on 9/25/2024.
//

#include "TimelineSemaphore.h"
#include "QueueFamily.h"
#include "Core/DispatchSystem/SystemHub.h"


namespace rhi {
    TimelineSemaphore::TimelineSemaphore() : graphicsQueue(yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics, 0)) {
        auto typeCreateInfo = vk::SemaphoreTypeCreateInfo()
                .setSemaphoreType(vk::SemaphoreType::eTimeline)
                .setInitialValue(value);
        auto semaphoreCreateInfo = vk::SemaphoreCreateInfo()
                .setPNext(&typeCreateInfo);

        graphicsQueue = yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics, 0);

        handle = vot::create("create timeline") = [&]{
            return yic::systemHub.val<ev::pVkSetupContext>().device->createSemaphore(semaphoreCreateInfo);
        };
    }

    TimelineSemaphore::~TimelineSemaphore() = default;

    auto TimelineSemaphore::submit(const vot::SubmitInfo& submitInfo) -> void {
        auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(submitInfo.waitValues)
                .setSignalSemaphoreValues(submitInfo.signalValues);

        auto sub = vk::SubmitInfo()
                .setCommandBuffers(submitInfo.cmds);

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
        auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(submitInfo.waitValues)
                .setSignalSemaphoreValues(submitInfo.signalValues);

        vk::Semaphore wait[] = {handle, submitInfo.waitSemaphore};
        vk::Semaphore signal[] = {handle, submitInfo.signalSemaphore};

        auto sub = vk::SubmitInfo()
                .setCommandBuffers(submitInfo.cmds)
                .setWaitSemaphores(wait)
                .setWaitDstStageMask(submitInfo.waitStageMasks)
                .setSignalSemaphores(signal)
                .setPNext(&timelineSubmitInfo);

        vot::SubmitInfo::increase();

        graphicsQueue.submit(sub, submitInfo.cmds.front().fence);
        return graphicsQueue.presentKHR(vk::PresentInfoKHR()
                                                .setSwapchains(swapchainKhr)
                                                .setImageIndices(imageIndex)
                                                .setWaitSemaphores(submitInfo.signalSemaphore));
    }

    auto TimelineSemaphore::clear() -> void {
        yic::systemHub.val<ev::pVkSetupContext>().device->destroy(handle);
    }
} // rhi
