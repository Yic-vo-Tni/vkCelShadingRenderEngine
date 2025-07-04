//
// Created by lenovo on 9/24/2024.
//

#include "Engine.h"
#include "atomic"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/QueueFamily.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

Engine::Engine() {
    mWindow = std::make_unique<yic::Window>(2000, 1200, "Yicvot");
}

Engine::~Engine() {
    std::unique_lock<std::mutex> lock(mMutex);
    mCondVar.wait(lock, [this]{ return mDestroy.load(); });

    yic::qFamily->acquireQueueUnSafe(vot::queueType::eGraphics).waitIdle();
    yic::systemHub.val<ev::pVkSetupContext>().device->waitIdle();

    yic::shaderHot->destroy();
    mEcs.reset();
    mRhi.reset();
    mWindow.reset();
}

auto Engine::run() -> void {
    mRenderThread = std::make_unique<std::thread>([this]{
        yic::shaderHot = ui::ShaderHotReload::make();

        mRhi = std::make_unique<rhi::Rhi>();
        mEcs = std::make_unique<sc::Ecs>();

        while (!mWindow->shouldClose().load(std::memory_order_relaxed)){
            mEcs->prepare();
            mRhi->render();
        }
        mWindow->renderClosed();
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mDestroy.store(true);
            mCondVar.notify_one();
        }
    });

        mWindow->loop([&]{ yic::systemHub.process(); });

        if (mRenderThread && mRenderThread->joinable())
            mRenderThread->join();

}


