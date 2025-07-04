//
// Created by lenovo on 10/14/2024.
//

#include "Ecs.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Camera/Camera.h"
#include "RHI/QueueFamily.h"
#include "Editor/ImGuiHub.h"
#include "RHI/DescriptorSystem.h"
#include "RS/ResourceSystem.h"
#include "SM/Scene.h"
#include "System/RenderLibrary.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

#include "oneapi/tbb/tick_count.h"


namespace sc {

    Ecs::Ecs() {
   //     configure();
        prepose();
        buildGlobalCamera();
    }

    Ecs::~Ecs() {
        yic::logger->warn("~ ecs");

        RenderLibrary::destroy();
        rs::ResourceSystem::destroy();
        sm::SceneSystem::destroy();
    };

//    auto Ecs::configure() -> void {}

    auto Ecs::prepose() -> void {
        ct = yic::systemHub.val<ev::pVkSetupContext>();
        rt = yic::systemHub.val<ev::pVkRenderContext>();

        yic::systemHub.sto(ev::freeCameraController{false, false, false, false, false, false, false});

        yic::resourceSystem = rs::ResourceSystem::make(ecs);
        yic::sceneSystem = sm::SceneSystem::make(ecs);
        yic::renderLibrary = RenderLibrary::make();

        inspectorPanel = std::make_unique<InspectorPanel>(ecs);
        submissionSystem = std::make_unique<RenderSubmissionSystem>(ecs);
    }

    auto Ecs::prepare() -> void {
        yic::systemHub.poll<ev::tModelLoaded>();

        oneapi::tbb::parallel_invoke(
                [&] {
                    handleCameraMovement(ecs.get<sc::Camera>(GLOBAL::camera));
                    yic::shaderHot->frame();
                    inspectorPanel->frame();
                },
                [&] {
                    yic::resourceSystem->frame();
                }
        );

        yic::sceneSystem->frame();
        submissionSystem->frame();
    }

    auto Ecs::buildGlobalCamera() -> void {
        GLOBAL::camera = ecs.create();
        auto& cam = ecs.emplace<sc::Camera>(GLOBAL::camera);
        cam.computeViewProjMatrix();

        cam.DS = yic::desSystem->allocUpdateDescriptorSets([&]{
            return vot::DescriptorLayout2{ cam.vpBufferInfo() };
        }, yic::renderLibrary->GP_Basic, 0, 1);
    }

    auto Ecs::handleCameraMovement(auto &cameraEntity) -> void {
        sc::Camera& c = cameraEntity;
        auto& f = yic::systemHub.val<ev::freeCameraController>();
        if (f.W == true)
            c.getPosition() += 0.1f * c.getCameraFront();
        if (f.S == true)
            c.getPosition() -= 0.1f * c.getCameraFront();
        if (f.A == true)
            c.getPosition() -= 0.1f * glm::normalize(glm::cross(c.getCameraFront(), c.getCameraUp()));
        if (f.D == true)
            c.getPosition() += 0.1f * glm::normalize(glm::cross(c.getCameraFront(), c.getCameraUp()));
        if (f.cursor == true)
            c.mouseCallback(f.xPos.value(), f.yPos.value());
        if (f.scroll == true)
            c.scrollCallback(f.xOffset.value(), f.yOffset.value());
        if (f.firstM == true)
            c.firstMouse = true;

        yic::systemHub.sto(ev::freeCameraController{false, false, false, false, false, false, false});

        c.computeViewProjMatrix();
    }

    auto Ecs::calFnTimeConsuming(const std::function<void()> &fn) -> void {
        auto b = oneapi::tbb::tick_count::now();

        fn();

        yic::logger->info("consuming: {0}", (oneapi::tbb::tick_count::now() - b).seconds() * 1000.0);
    }


} // sc




