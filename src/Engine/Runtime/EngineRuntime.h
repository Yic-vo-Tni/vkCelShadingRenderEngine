//
// Created by lenovo on 10/14/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_H
#define VKCELSHADINGRENDERER_ECS_H

#include "RHI/GpuRuntime.h"
#include "System/InspectorPanel.h"
#include "System/RenderSubmissionSystem.h"

namespace sc { //rename runtime

    class EngineRuntime {
    public:
        EngineRuntime();
        ~EngineRuntime();

        auto tickF() -> void;
        auto tickS() -> void;
        auto tickR() -> void;
    private:
        auto prepose() -> void;
    private:
        entt::registry ecs;

        ev::pVkSetupContext ct{};
        ev::pVkRenderContext rt{};

        std::unique_ptr<rhi::GpuRuntime> gpuRuntime;
        std::unique_ptr<InspectorPanel> inspectorPanel;
        std::unique_ptr<RenderSubmissionSystem> submissionSystem;
    };

} // sc

#endif //VKCELSHADINGRENDERER_ECS_H
