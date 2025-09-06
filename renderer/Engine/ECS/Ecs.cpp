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

        ct.device->waitIdle();
        ecs.clear<>();
        RenderLibrary::destroy();
        rs::ResourceSystem::destroy();
        sm::SceneSystem::destroy();
    };

    auto Ecs::prepose() -> void {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();

        // yic::systemHub.sto(ev::freeCameraController{false, false, false, false, false, false, false});

        yic::resourceSystem = rs::ResourceSystem::make(ecs);
        yic::sceneSystem = sm::SceneSystem::make(ecs);
        yic::renderLibrary = RenderLibrary::make();

        inspectorPanel = std::make_unique<InspectorPanel>(ecs);
        submissionSystem = std::make_unique<RenderSubmissionSystem>(ecs);
    }

    auto Ecs::render() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded>();

        yic::indexRing.get(vot::LogicBufferType::eFast).read_begin();
        yic::indexRing.get(vot::LogicBufferType::eSlow).read_begin();
        {
            yic::shaderHot->frame();
            inspectorPanel->frame();

            yic::sceneSystem->frame();
            submissionSystem->frame();
        }
    }

    auto Ecs::fastLogic() -> void {
        const auto fastW = yic::indexRing.get(vot::LogicBufferType::eFast).write_begin();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (fastW == 0xff) return;

        {
            updateCamera(ecs.get<sc::Camera>(GLOBAL::camera), fastW);
        }

        yic::indexRing.get(vot::LogicBufferType::eFast).write_end();
    }

    auto Ecs::slowLogic() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded>();
        const auto slowW = yic::indexRing.get(vot::LogicBufferType::eSlow).write_begin();

        static bool firstRun = true;
        if (firstRun) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            firstRun = false;
        }

        if (slowW == 0xff ) return;

        {
            yic::resourceSystem->frame();
        }

        yic::indexRing.get(vot::LogicBufferType::eSlow).write_end();
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

    auto Ecs::updateCamera(auto &cameraEntity, auto& i) -> void {
        sc::Camera& c = cameraEntity;
        {
            auto f_Lock = yic::systemHub.vaL<ev::vFreeCameraController>();
            if (f_Lock->W == true) c.getPosition() += 0.1f * c.getCameraFront();
            if (f_Lock->S == true) c.getPosition() -= 0.1f * c.getCameraFront();
            if (f_Lock->A == true) c.getPosition() -= 0.1f * glm::normalize(glm::cross(c.getCameraFront(), c.getCameraUp()));
            if (f_Lock->D == true) c.getPosition() += 0.1f * glm::normalize(glm::cross(c.getCameraFront(), c.getCameraUp()));
            if (f_Lock->cursor == true) c.mouseCallback(f_Lock->xPos, f_Lock->yPos);
            if (f_Lock->scroll == true) c.scrollCallback(f_Lock->xOffset, f_Lock->yOffset);
            if (f_Lock->firstM == true) c.firstMouse = true;

            f_Lock->W = false;
            f_Lock->S = false;
            f_Lock->A = false;
            f_Lock->D = false;
            f_Lock->cursor = false;
            f_Lock->scroll = false;
            f_Lock->firstM = false;
        }

        c.updateCamera(i);
    }

    auto Ecs::calFnTimeConsuming(const std::function<void()> &fn) -> void {
        const auto b = oneapi::tbb::tick_count::now();

        fn();

        yic::logger->info("consuming: {0}", (oneapi::tbb::tick_count::now() - b).seconds() * 1000.0);
    }


} // sc




