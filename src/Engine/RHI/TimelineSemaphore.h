//
// Created by lenovo on 9/25/2024.
//

#ifndef VKCELSHADINGRENDERER_TIMELINESEMAPHORE_H
#define VKCELSHADINGRENDERER_TIMELINESEMAPHORE_H


namespace rhi {

    class TimelineSemaphore {
    public:
        Make = []{ return Singleton<TimelineSemaphore>::make_ptr(); };
        TimelineSemaphore();
        ~TimelineSemaphore();

        auto submit(const vot::SubmitInfo& submitInfo) -> void;
        auto finalSubmit(vk::SwapchainKHR swapchainKhr, uint32_t imageIndex, const vot::SubmitInfo& submitInfo) -> vk::Result;

        auto clear() -> void;
    private:
        uint64_t value{0};
        vk::Semaphore handle{};
        vk::Queue graphicsQueue{};
    };

} // rhi

namespace yic{
    inline rhi::TimelineSemaphore* timeline;
}

#endif //VKCELSHADINGRENDERER_TIMELINESEMAPHORE_H
