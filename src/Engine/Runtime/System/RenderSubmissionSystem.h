//
// Created by lenovo on 5/20/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
#define VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H

#include "RenderFlow/RenderGraph.h"
#include "RenderStage/RenderStage.h"
#include "DER/DynamicEditableRendering.h"

namespace sc {

    class RenderSubmissionSystem {
    public:
        explicit RenderSubmissionSystem(entt::registry& registry);
        ~RenderSubmissionSystem();

        auto frame() -> void;
    private:
        entt::registry& ecs;
        std::unique_ptr<runtime::flow::RenderGraph> uRenderGraph;
        std::unique_ptr<runtime::flow::DynamicEditableRendering> uDynamicEditableRendering;
        std::unique_ptr<RenderStage> uRenderStage;
    };

} // sc

#endif //VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
