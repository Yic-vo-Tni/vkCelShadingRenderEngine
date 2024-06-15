//
// Created by lenovo on 5/23/2024.
//

#include "Engine.h"

namespace yic {

    Engine::Engine() = default;

    bool Engine::run() {
        ShaderFolderWatcher::start();

        mFrameLoopThread = std::make_unique<std::thread>([this]{
           mRhi = std::make_unique<vkRhi>();

            while (mFrameLoop.load()){
                SemaphoreGuard guard(mFrameLoop_semaphore);
                TaskBus::executeTask<tt::EngineFlow>(true);
            }
        });

        if(vkWindow::run()){
            mFrameLoop_semaphore.release();
            mFrameLoop.exchange(false);
        }

        if (mFrameLoopThread && mFrameLoopThread->joinable())
            mFrameLoopThread->join();
        ShaderFolderWatcher::end();

        return true;
    }

} // yic