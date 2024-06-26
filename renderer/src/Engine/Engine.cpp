//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    Engine::Engine() = default;

    bool Engine::run() {
        ShaderFolderWatcher::start();

        if ([]{
            int cpuInfo[4] = {-1};
            __cpuid(cpuInfo, 0);
            int nIds = cpuInfo[0];
            if(nIds >= 7){
                __cpuidex(cpuInfo, 7, 0);
                bool tsxSupported = (cpuInfo[1] & ((1 << 11) | (1 << 4))) != 0;
                return tsxSupported;
            }
            return false;
        }){
            vkInfo("TSX is supported");
        } else { vkWarn("TSX is not supported"); }

        mRhi = std::make_unique<vkRhi>();
        mFrameLoopThread = std::make_unique<std::thread>([this]{
            while (mFrameLoop.load()){
                TaskBus::executeTaskSpecific(tt::RenderTarget_s::eImGuiWindow, {}, true);
            }
        });

        vkWindow::run();
        mFrameLoop_semaphore.release();
        mFrameLoop.exchange(false);

        if (mFrameLoopThread && mFrameLoopThread->joinable())
            mFrameLoopThread->join();
        ShaderFolderWatcher::end();

        return true;
    }

} // yic