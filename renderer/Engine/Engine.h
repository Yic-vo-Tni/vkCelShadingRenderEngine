//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_ENGINE_H
#define VKCELSHADINGRENDERER_ENGINE_H

#include "Editor/Window.h"
#include "ECS/Ecs.h"
#include "RHI/Rhi.h"

class Engine {
public:
    Engine();
    ~Engine();

    auto run() ->  void ;
private:
    std::unique_ptr<yic::Window> mWindow;

    std::unique_ptr<rhi::Rhi> mRhi;
    std::unique_ptr<sc::Ecs> mEcs;
    std::atomic_bool mLoopStop = true;
    std::unique_ptr<std::thread> mRenderThread;

    std::atomic_bool mDestroy = false;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};


#endif //VKCELSHADINGRENDERER_ENGINE_H
