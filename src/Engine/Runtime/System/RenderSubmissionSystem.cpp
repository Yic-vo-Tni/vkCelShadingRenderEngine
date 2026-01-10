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
        uRenderGraph = std::make_unique<runtime::flow::RenderGraph>();
        uRenderStage = std::make_unique<RenderStage>(ecs);
        uDynamicEditableRendering = std::make_unique<runtime::flow::DynamicEditableRendering>();
        yic::derTranslator = runtime::flow::DERTranslator::make(uDynamicEditableRendering.get());
       // yic::dispatchHelper = runtime::flow::DispatchHelper::make(ecs);
    }

    RenderSubmissionSystem::~RenderSubmissionSystem() = default;

    auto RenderSubmissionSystem::frame() -> void {
        // yic::dispatchHelper->frame();
        // uDynamicEditableRendering->drawEditor();
        //
        // using namespace runtime::flow;
        // RG_DSL::ctx(uRenderGraph)
        //
        // | RG_DSL::begin
        //
        // | RG_DSL::lambda([&](auto &g) {
        //     for (auto &[n, nodes]: uDynamicEditableRendering->acquireRF().algorithms) {
        //         for (auto &node: nodes) {
        //             g = std::move(g) |
        //                 std::visit([&]<typename T0>(T0 &&v) {
        //                     using T = std::decay_t<T0>;
        //                     if constexpr (std::is_same_v<T, vot::Image_sptr>) {
        //                         return PassNode{v};
        //                     } else if constexpr (std::is_same_v<T, vot::string>) {
        //                         return PassNode{v};
        //                     }
        //                 }, node.makePassNode())
        //                 + Node::read{node.reads}
        //                 >> Node::invoke{node.dispatch};
        //         }
        //     }
        // })
        //
        // | RG_DSL::end;

        /////////////////////////////////////////////////////////

        uRenderStage->update(); //
        auto& RL = yic::renderLibrary;


        using namespace runtime::flow;
        RG_DSL::ctx(uRenderGraph)
        | RG_DSL::begin

        | PassNode{"comp_skinning"}
        >> Node::invoke{RG_STAGE(comp_skinning)}

        | PassNode{RL->RT_GBuffer}
        >> Node::invoke{RG_STAGE(drawing_gBuffer)}

        | PassNode{RL->RT_ShadowMap}
        + Node::read{RL->RT_GBuffer}
        >> Node::invoke{RG_STAGE(drawing_shadowMap)} //FIXME: lie over

        | PassNode{RL->RT_IDBuffer}
        + Node::read{RL->RT_GBuffer}
        >> Node::invoke{RG_STAGE(drawing_IDBuffer)}

        | PassNode{RL->RT_Volumetric_Clouds}
        + Node::read{RL->RT_GBuffer}
        >> Node::invoke{RG_STAGE(drawing_volumetricClouds)}

        | PassNode{RL->RT_Volumetric_Fog}
        + Node::read{RL->RT_GBuffer}
        >> Node::invoke{RG_STAGE(drawing_volumetricFog)} //FIXME: no effect

        | PassNode{RL->RTX_RayTracing}
        >> Node::invoke{RG_STAGE(draw_RTShadow)}

        | PassNode{RL->RT_Post}
        + Node::read{RL->RT_Volumetric_Clouds, RL->RT_GBuffer, RL->RTX_RayTracing}
        >> Node::invoke{RG_STAGE(drawing_post)}

        | RG_DSL::end;
    }


} // sc