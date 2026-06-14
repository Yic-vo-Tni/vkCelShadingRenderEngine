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
    {
        std::unique_lock lock(mMutex);
        mCondVar.wait(lock, [this] { return mDestroy.load(); });

        yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics).waitIdle();
        yic::systemHub.va<ev::pVkSetupContext>().device->waitIdle();

        ui::ShaderHotReload::destroy();
        mEngineRuntime.reset();
        mWindow.reset();

        if (mRenderThread && mRenderThread->joinable()) mRenderThread->join();
    }

    // yic::logger->info("before return main");
    //MessageBoxA(nullptr, "ABOUT TO EXITPROCESS", "DBG", MB_OK);

    yic::logger->info("before ExitProcess");
    //dumpLoadedModules();

    //test
    // auto* fake = reinterpret_cast<void*>(0x1234);
    // HF_TRACK_RESOURCE(vot::core::diag::eRuntimeSystem, fake);
    //HF_UNTRACK_RESOURCE(fake);

    vot::core::instance::lifetimeTracker().dump();

    if (!vot::core::instance::lifetimeTracker().empty()) {
        yic::logger->error("[ShutdownAudit] engine resources are still alive.");
        yic::logger->flush();
        std::abort();
    }

    yic::logger->info("[ShutdownAudit] engine lifetime audit passed.");
    yic::logger->flush();

    // FIXME:
    // ExitProcess may hang during external DLL detach,
    // e.g. OBS/NVIDIA overlay/libwinpthread.
    // Engine-side shutdown has completed before this point.
    TerminateProcess(GetCurrentProcess(), 0);
    //ExitProcess(0);
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



