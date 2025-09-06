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
                GLOBAL::fps = frameCount;
                std::cout << "FPS: " << frameCount << std::endl;
                frameCount = 0;
                lastTime = currentTime;
            }
        }

    private:
        float frameCount;
        std::chrono::high_resolution_clock::time_point lastTime;
    };

    class Rhi {
    public:
        Rhi();
        ~Rhi();

        auto render() -> void;
    private:
        std::unique_ptr<VkInit> mVkInit;
        std::unique_ptr<Swapchain> mSwapchain;
        std::unique_ptr<FrameRate> mFrameRate;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_RHI_H
