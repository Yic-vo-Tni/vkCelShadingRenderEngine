//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    Engine::Engine() = default;

    bool Engine::run() {
        ShaderFolderWatcher::start();

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