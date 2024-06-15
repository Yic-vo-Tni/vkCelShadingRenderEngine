//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_ENGINE_H
#define VKCELSHADINGRENDERER_ENGINE_H

#include "Engine/Core/vkWindow.h"
#include "Engine/Core/FileOperator/ShaderHotReloader.h"
#include "Engine/RHI/vkRhi.h"

namespace yic {

    class Engine {
    public:
        Engine();
        bool run();

    private:
        std::unique_ptr<vkRhi> mRhi;

        std::unique_ptr<std::thread> mFrameLoopThread;
        std::atomic<bool> mFrameLoop{true};
        std::counting_semaphore<1> mFrameLoop_semaphore{1};
    };

} // yic

#endif //VKCELSHADINGRENDERER_ENGINE_H
