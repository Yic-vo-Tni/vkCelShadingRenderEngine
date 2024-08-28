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
        }()){
            vkInfo("TSX is supported");
        } else { vkWarn("TSX is not supported"); }

        auto numThreads = std::thread::hardware_concurrency();
        vkInfo("Max threads counts: {0}", numThreads);


        mFrameLoopThread = std::make_unique<std::thread>([this]{
            mRhi = std::make_unique<vkRhi>();
            while (mFrameLoop.load()){
                TaskBus::executeTaskSpecific(tt::RenderTarget_s::eMainWindow, {}, true);
            }
        });

        vkWindow::run();
        mFrameLoop_semaphore.release();
        mFrameLoop.exchange(false);

        if (mFrameLoopThread && mFrameLoopThread->joinable())
            mFrameLoopThread->join();
        ShaderEditor::end();
        ShaderFolderWatcher::end();

        return true;
    }

} // yic