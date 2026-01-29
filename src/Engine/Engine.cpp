//
// Created by lenovo on 9/24/2024.
//

#include "Engine.h"
#include "atomic"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/QueueFamily.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

Engine::Engine() {
    mWindow = std::make_unique<yic::Window>("Hakuro Fabric");
    yic::shaderHot = ui::ShaderHotReload::make();
    mEngineRuntime = std::make_unique<sc::EngineRuntime>();
}

Engine::~Engine() {
    std::unique_lock lock(mMutex);
    mCondVar.wait(lock, [this]{ return mDestroy.load(); });

    yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics).waitIdle();
    yic::systemHub.va<ev::pVkSetupContext>().device->waitIdle();

    ui::ShaderHotReload::destroy();
    mEngineRuntime.reset();
    mWindow.reset();

    yic::logger->info("before return main");
    //MessageBoxA(nullptr, "ABOUT TO EXITPROCESS", "DBG", MB_OK);
    ExitProcess(0);
}

auto Engine::run() -> void {
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

    mFastLogicThread = Launch(
        THREAD_PRIORITY_NORMAL,
        [this] { mEngineRuntime->tickF(); }
    );
    mSlowLogicThread = Launch(
        THREAD_PRIORITY_HIGHEST,
        [this] { mEngineRuntime->tickS(); }
    );
    mRenderThread = Launch(
        THREAD_PRIORITY_HIGHEST,
        [this] { mEngineRuntime->tickR(); },
        [this] {
            mWindow->renderClosed();
            std::lock_guard lock(mMutex);
            mDestroy.store(true);
            mCondVar.notify_one();
        }
    );

    mWindow->loop([&]{ yic::systemHub.process(); });

    if (mRenderThread && mRenderThread->joinable()) mRenderThread->join();
    if (mFastLogicThread && mFastLogicThread->joinable()) mFastLogicThread->join();
    if (mSlowLogicThread && mSlowLogicThread->joinable()) mSlowLogicThread->join();
}



