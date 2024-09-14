//
// Created by lenovo on 9/14/2024.
//

#ifndef VKCELSHADINGRENDERER_TIMELINESEMAPHOREMANAGER_H
#define VKCELSHADINGRENDERER_TIMELINESEMAPHOREMANAGER_H


namespace TimeLine{
    const inline uint64_t eInit_phase_start = 0;
    const inline uint64_t eInit_phase_end = 99;
    const inline uint64_t eRender_phase_start = 100;
    const inline uint64_t eRender_phase_end = 190;
    const inline uint64_t eReset = 200;
}

namespace yic {

    class TimelineSemaphoreManager {
    public:
        vkGet auto make = []{ return Singleton<TimelineSemaphoreManager>::get(); };
        TimelineSemaphoreManager();
        ~TimelineSemaphoreManager();

        auto signal(const uint64_t& v) -> void;
        auto wait(uint64_t v) -> void;
        auto submit(const vk::Queue& queue, uint64_t waitValue, uint64_t signalValue) -> void;
        auto finalSubmit(const vk::Queue& queue, uint64_t waitValue) -> void;
        auto clear() -> void;

        auto& acquire() { return mSemaphore; }
    private:
        auto createSemaphore() -> void;
    private:
        uint64_t mCounter{0};
        uint64_t mAddValue{0};
        vk::Device mDevice;
        vk::Semaphore mSemaphore;
    };

} // yic

namespace mg{
    inline yic::TimelineSemaphoreManager* TimelineSemaphoreManager;
}

#endif //VKCELSHADINGRENDERER_TIMELINESEMAPHOREMANAGER_H
