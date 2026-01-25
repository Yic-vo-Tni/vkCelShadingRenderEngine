//
// Created by lenovo on 10/14/2024.
//

#include "EngineRuntime.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
#include "Camera/Camera.h"
#include "RS/ResourceSystem.h"
#include "SM/Scene.h"
#include "System/RenderLibrary.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

namespace sc {

    EngineRuntime::EngineRuntime() {
        vot::EntityView<vot::bit::eEntityViewDefault>::Init(ecs); // TODO: if more scene(entt::register)
        prepose();
    }

    EngineRuntime::~EngineRuntime() {
        yic::logger->warn("~ ecs");

        ct.device->waitIdle();

        yic::systemHub.pub(ev::tDestroyVMA{});
        yic::systemHub.setEvent(ev::tModelLoaded{});
        yic::systemHub.pub(ev::tDestroyVMA{});

        RenderLibrary::destroy();
        rs::ResourceSystem::destroy();
        sm::SceneSystem::destroy();

        submissionSystem.reset();
        inspectorPanel.reset();

        ecs.clear();
        gpuRuntime.reset();
    }

    auto EngineRuntime::prepose() -> void {
        gpuRuntime = std::make_unique<rhi::GpuRuntime>();

        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();

        yic::renderLibrary = RenderLibrary::make();
        yic::resourceSystem = rs::ResourceSystem::make(ecs);
        yic::sceneSystem = sm::SceneSystem::make(ecs);

        inspectorPanel = std::make_unique<InspectorPanel>(ecs);
        submissionSystem = std::make_unique<RenderSubmissionSystem>(ecs);
    }

    auto EngineRuntime::tickR() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded, ev::tDestroyEntity>();

        yic::indexRing.read([&] {
            yic::shaderHot->frame();
            inspectorPanel->frame();

            yic::sceneSystem->frame();
            submissionSystem->frame(); // TODO: Split into -> RHI thread

            gpuRuntime->present();
        }, vot::eFast, vot::eSlow);
    }

    auto EngineRuntime::tickF() -> void {
        yic::indexRing.write([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // TODO: Take on some takes in future

            Camera::UpdateUnique();
        }, vot::eFast);
    }

    auto EngineRuntime::tickS() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded, ev::tDestroyEntity>();

        yic::indexRing.write([&] {
            yic::resourceSystem->frame(); // TODO: Split into -> Animation.frame()
        }, vot::eSlow);
    }


} // sc




