//
// Created by lenovo on 9/14/2024.
//

#include "TimelineSemaphoreManager.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {


    TimelineSemaphoreManager::TimelineSemaphoreManager() {
        auto& ct = mg::SystemHub.val<ev::pVkSetupContext>();
        mDevice = *ct.device;

        createSemaphore();
    }

    TimelineSemaphoreManager::~TimelineSemaphoreManager() = default;

    auto TimelineSemaphoreManager::signal(const uint64_t &v) -> void {
        auto signalInfo = vk::SemaphoreSignalInfo()
                .setSemaphore(mSemaphore)
                .setValue(v + mAddValue);
        mDevice.signalSemaphore(signalInfo);
    }

    auto TimelineSemaphoreManager::wait(uint64_t v) -> void {
        v += mAddValue;
        auto waitInfo = vk::SemaphoreWaitInfo()
                .setSemaphores(mSemaphore)
                .setValues(v);
        if (mDevice.waitSemaphores(waitInfo, UINT64_MAX) != vk::Result::eSuccess){
            throw std::runtime_error( "failed to waite the timeline sem value: " + std::to_string(v));
        };
    }

    auto TimelineSemaphoreManager::createSemaphore() -> void {
        auto timelineCreateInfo = vk::SemaphoreTypeCreateInfo()
                .setSemaphoreType(vk::SemaphoreType::eTimeline)
                .setInitialValue(0);

        auto createInfo = vk::SemaphoreCreateInfo()
                .setPNext(&timelineCreateInfo);

        mSemaphore = mDevice.createSemaphore(createInfo);
    }

    auto TimelineSemaphoreManager::submit(const vk::Queue &queue, uint64_t waitValue, uint64_t signalValue) -> void {
        waitValue += mAddValue;
        signalValue += mAddValue;
        auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(waitValue)
                .setSignalSemaphoreValues(signalValue);

        auto submitInfo = vk::SubmitInfo()
                .setWaitSemaphores(mSemaphore)
                .setSignalSemaphores(mSemaphore)
                .setPNext(&timelineSubmitInfo);

        queue.submit(submitInfo);
    }

    auto TimelineSemaphoreManager::finalSubmit(const vk::Queue &queue, uint64_t waitValue) -> void {
        waitValue += mAddValue;
        mAddValue += TimeLine::eReset;
        auto timelineSubmitInfo = vk::TimelineSemaphoreSubmitInfo()
                .setWaitSemaphoreValues(waitValue)
                .setSignalSemaphoreValues(mAddValue);

        auto submitInfo = vk::SubmitInfo()
                .setWaitSemaphores(mSemaphore)
                .setSignalSemaphores(mSemaphore)
                .setPNext(&timelineSubmitInfo);

        queue.submit(submitInfo);
        ++mCounter;
    }

    auto TimelineSemaphoreManager::clear() -> void {
        mDevice.destroy(mSemaphore);
    }


} // yic