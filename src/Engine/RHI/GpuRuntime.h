//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_RHI_H
#define VKCELSHADINGRENDERER_RHI_H

#include "Common.h"
#include "VkInit.h"
#include "Swapchain.h"

namespace rhi {

    class FrameRate {
    public:
        FrameRate() : frameCount(0), lastTime(std::chrono::high_resolution_clock::now()) {}

        void update() {
            auto currentTime = std::chrono::high_resolution_clock::now();
            frameCount++;

            if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count() >= 1) {
                //GLOBAL::fps = frameCount;
                GLOBAL::fps = std::max(frameCount, 1.0f);
                std::cout << "FPS: " << frameCount << std::endl;
                frameCount = 0;
                lastTime = currentTime;
            }
        }

    private:
        float frameCount;
        std::chrono::high_resolution_clock::time_point lastTime;
    };

    class TimeSystem {
    public:
        TimeSystem() : lastFrameTime(clock::now()), lastFpsTime(clock::now()), frameCount(0), deltaTime(0.f), fps(0.f){}

        auto update() -> void {
            const auto now = clock::now();

            deltaTime = std::chrono::duration<float>(now - lastFrameTime).count();
            lastFrameTime = now;

            deltaTime = std::min(deltaTime, 0.05f);

            frameCount++;

            if (const float fpsElapsed = std::chrono::duration<float>(now - lastFpsTime).count(); fpsElapsed >= 1.f) {
                fps = frameCount / fpsElapsed;
                frameCount = 0;
                lastFpsTime = now;
                //std::cout << "FPS: " << GLOBAL::fps << ", dt: " << GLOBAL::dt << std::endl;
                yic::logger->trace("fps: {0}, dt: {1}", GLOBAL::fps, GLOBAL::dt.load(std::memory_order_relaxed));
            }

            GLOBAL::fps = fps;
//            GLOBAL::dt = deltaTime;
            GLOBAL::dt.store(deltaTime, std::memory_order_relaxed);

        }
    private:
        using clock = std::chrono::steady_clock;

        clock::time_point lastFrameTime;
        clock::time_point lastFpsTime;
        std::uint32_t frameCount;
        float deltaTime;
        float fps;
    };

    class GpuRuntime {
    public:
        GpuRuntime();
        ~GpuRuntime();

        auto present() -> void;
    private:
        std::unique_ptr<VkInit> mVkInit;
        std::unique_ptr<Swapchain> mSwapchain;
        //std::unique_ptr<FrameRate> mFrameRate;
        TimeSystem mTimeSystem;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_RHI_H
