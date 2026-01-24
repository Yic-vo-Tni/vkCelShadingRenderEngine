//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_ENGINE_H
#define VKCELSHADINGRENDERER_ENGINE_H

#include "Editor/Window.h"
#include "Runtime/EngineRuntime.h"

class Engine {
public:
    Engine();
    ~Engine();

    auto run() ->  void ;
private:
    std::unique_ptr<yic::Window> mWindow;

    std::unique_ptr<sc::EngineRuntime> mEngineRuntime;
    std::atomic_bool mLoopStop = true;
    std::unique_ptr<std::thread> mRenderThread;
    std::unique_ptr<std::thread> mFastLogicThread;
    std::unique_ptr<std::thread> mSlowLogicThread;

    std::mutex mInitMutex;
    std::condition_variable mInitCondVar;
    bool mInit = false;

    std::atomic_bool mDestroy = false;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};


#endif //VKCELSHADINGRENDERER_ENGINE_H
