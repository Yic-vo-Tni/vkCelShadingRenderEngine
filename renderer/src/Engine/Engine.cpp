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
                TaskBus::executeTask<tt::EngineFlow>();
            }
        });

        if(vkWindow::run()){
            mFrameLoop.exchange(false);
        }

        if (mFrameLoopThread->joinable())
            mFrameLoopThread->join();
        ShaderFolderWatcher::end();

        return true;
    }

} // yic