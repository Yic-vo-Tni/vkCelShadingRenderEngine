//
// Created by lenovo on 5/20/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
#define VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H

#include "RenderGraph.h"
#include "RenderStage/RenderStage.h"

namespace sc {

    class RenderSubmissionSystem {
    public:
        explicit RenderSubmissionSystem(entt::registry& registry);
        ~RenderSubmissionSystem();

        auto frame() -> void;
    private:
        entt::registry& ecs;
        std::unique_ptr<RenderGraph> uRenderGraph;
        std::unique_ptr<RenderStage> uRenderStage;
    };

} // sc

#endif //VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
