//
// Created by lenovo on 10/14/2024.
//

#include "Ecs.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
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
        prepose();
        buildGlobalCamera();
    }

    Ecs::~Ecs() {
        yic::logger->warn("~ ecs");

        RenderLibrary::destroy();
        rs::ResourceSystem::destroy();
        sm::SceneSystem::destroy();
    };

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
//        yic::systemHub.poll<ev::tModelLoaded>();
//
//        handleCameraMovement(ecs.get<sc::Camera>(GLOBAL::camera));
//        yic::shaderHot->frame();
//        inspectorPanel->frame();
//        yic::resourceSystem->frame();
//        yic::sceneSystem->frame();
//        submissionSystem->frame();
    }

    auto Ecs::render() -> void {
        yic::systemHub.poll<ev::tModelLoaded>();

        auto i = yic::indexRing.get(vot::LogicBufferType::eFast).read_begin();

        {
            oneapi::tbb::parallel_invoke(
                    [&] {
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
        yic::indexRing.get(vot::LogicBufferType::eFast).read_end();
    }

    auto Ecs::fastLogic() -> void {
        auto i = yic::indexRing.get(vot::LogicBufferType::eFast).write_begin();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (i == 0xff) return;
        {
            handleCameraMovement(ecs.get<sc::Camera>(GLOBAL::camera), i);
        }
        yic::indexRing.get(vot::LogicBufferType::eFast).write_end();
    }

    auto Ecs::slowLogic() -> void {

    }

    auto Ecs::buildGlobalCamera() -> void {
        GLOBAL::camera = ecs.create();
        auto& cam = ecs.emplace<sc::Camera>(GLOBAL::camera);
        cam.computeViewProjMatrix();

        GLOBAL::set0 = ecs.create();
        auto& set0 = ecs.emplace<vot::DescriptorSet0>(GLOBAL::set0);
        for(auto i = 0; i < 3; i++){
            set0.handles[i] = yic::desSystem->allocUpdateDescriptorSets([&]{
                return vot::DescriptorLayout2{ cam.vpBufferInfo(i) };
            }, yic::renderLibrary->GP_Basic_Assimp, 0, 1);
        }
    }

    auto Ecs::handleCameraMovement(auto &cameraEntity, auto& i) -> void {
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

        //c.computeViewProjMatrix();
        c.updateCamera(i);
    }

    auto Ecs::calFnTimeConsuming(const std::function<void()> &fn) -> void {
        auto b = oneapi::tbb::tick_count::now();

        fn();

        yic::logger->info("consuming: {0}", (oneapi::tbb::tick_count::now() - b).seconds() * 1000.0);
    }


} // sc




