//
// Created by lenovo on 10/14/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_H
#define VKCELSHADINGRENDERER_ECS_H

#include "System/InspectorPanel.h"
#include "System/RenderSubmissionSystem.h"

namespace sc {

    class Ecs {
    public:
        Ecs();
        ~Ecs();

        auto prepare() -> void;
    private:
//        auto configure() -> void;
        auto prepose() -> void;

    private:
        auto buildGlobalCamera() -> void;
        auto handleCameraMovement(auto& cameraEntity) -> void;
        static auto calFnTimeConsuming(const std::function<void()>& fn) -> void;
    private:
//        flecs::world ecs;
        entt::registry ecs;

        ev::pVkSetupContext ct{};
        ev::pVkRenderContext rt{};

        std::unique_ptr<InspectorPanel> inspectorPanel;
        std::unique_ptr<RenderSubmissionSystem> submissionSystem;
    };

} // sc

#endif //VKCELSHADINGRENDERER_ECS_H
