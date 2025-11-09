//
// Created by lenovo on 10/16/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERSTAGE_H
#define VKCELSHADINGRENDERER_RENDERSTAGE_H
#include "../RenderFlow/RenderGraph.h"

namespace sc {
    class RenderStage {
    public:
        explicit RenderStage(entt::registry& registry);
        ~RenderStage() = default;

        auto update() -> void;

        auto comp_skinning(vot::CommandBuffer& cmd) const -> void;

        auto drawing_gBuffer(vot::CommandBuffer& cmd) -> void;
        auto drawing_shadowMap(vot::CommandBuffer& cmd) -> void;
        auto drawing_IDBuffer(vot::CommandBuffer& cmd) -> void;
        auto drawing_volumetricClouds(vot::CommandBuffer& cmd) const -> void;
        auto drawing_volumetricFog(vot::CommandBuffer& cmd) const -> void;
        auto drawing_post(vot::CommandBuffer& cmd) const -> void;

        auto draw_RTShadow(vot::CommandBuffer& cmd) const -> void;
    private:
        entt::registry& ecs;
        ev::pVkSetupContext ct{};
        ev::pVkRenderContext rt{};
        float iTime{1.f};
        std::uint8_t fast{}, slow{};
        vot::DescriptorHandle set0{};
    };
} // sc

#endif //VKCELSHADINGRENDERER_RENDERSTAGE_H