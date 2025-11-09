//
// Created by lenovo on 5/20/2025.
//
#include "Core/Management/TripleBufferIndexManager.h"
#include "RenderSubmissionSystem.h"
#include "RenderLibrary.h"
#include "RS/ResourceSystem.h"


namespace sc {
    #define RG_STAGE(fn) std::bind_front(&RenderStage::fn, uRenderStage.get())

    RenderSubmissionSystem::RenderSubmissionSystem(entt::registry& registry) : ecs{registry}{
        uRenderGraph = std::make_unique<RenderGraph>();
        uRenderStage = std::make_unique<RenderStage>(ecs);
    }

    RenderSubmissionSystem::~RenderSubmissionSystem() = default;

    auto RenderSubmissionSystem::frame() -> void {
        uRenderStage->update();

        auto& RL = yic::renderLibrary;

        RG_DSL::ctx(uRenderGraph)

        | RG_DSL::begin

        | PassNode{}
        >> Node::exec{RG_STAGE(comp_skinning)}

        | PassNode{RL->RT_GBuffer}
        >> Node::exec{RG_STAGE(drawing_gBuffer)}

        | PassNode{RL->RT_ShadowMap}
        >> Node::exec{RG_STAGE(drawing_shadowMap)} //FIXME: lie over

        | PassNode{RL->RT_IDBuffer}
        >> Node::exec{RG_STAGE(drawing_IDBuffer)}

        | PassNode{RL->RT_Volumetric_Clouds}
        >> Node::exec{RG_STAGE(drawing_volumetricClouds)}

        | PassNode{RL->RT_Volumetric_Fog}
        + Node::in{RL->RT_GBuffer}
        >> Node::exec{RG_STAGE(drawing_volumetricFog)} //FIXME: no effect

        | PassNode{RL->RTX_RayTracing}
        + Node::ctx{uRenderStage->drawci_RTShadow()}
        >> Node::exec{RG_STAGE(draw_RTShadow)}

        | PassNode{RL->RT_Post}
        + Node::in{RL->RT_GBuffer}
        >> Node::exec{RG_STAGE(drawing_post)}

        | RG_DSL::end;
    }


} // sc