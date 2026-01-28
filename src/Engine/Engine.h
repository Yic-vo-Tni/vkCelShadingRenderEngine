//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_ENGINE_H
#define VKCELSHADINGRENDERER_ENGINE_H

#include "Editor/Window.h"
#include "Runtime/EngineRuntime.h"

class Engine {
    template<typename TickFn, typename ExitFn = std::nullptr_t>
    std::unique_ptr<std::thread> Launch(const int winPriority, TickFn&& tick, ExitFn&& onExit = nullptr) {
        return std::make_unique<std::thread>([=, this] {
           // {
           //     std::unique_lock lock(mInitMutex);
           //     mInitCondVar.wait(lock, [this]{ return mInit.load(std::memory_order_acquire); });
           // }

            SetThreadPriority(GetCurrentThread(), winPriority);

            while (!mWindow->shouldClose().load(std::memory_order_relaxed)) {
                tick();
            }

            if constexpr (!std::is_same_v<ExitFn, std::nullptr_t>) {
                onExit();
            }
        });
    }
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
    std::atomic<bool> mInit{false};

    std::atomic_bool mDestroy = false;
    std::mutex mMutex;
    std::condition_variable mCondVar;
};


#endif //VKCELSHADINGRENDERER_ENGINE_H


