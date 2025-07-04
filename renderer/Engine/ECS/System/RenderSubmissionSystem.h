//
// Created by lenovo on 5/20/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
#define VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H


namespace sc {

    class RenderSubmissionSystem {
    public:
        explicit RenderSubmissionSystem(entt::registry& registry);
        ~RenderSubmissionSystem();

        auto frame() -> void;
    private:
        auto flow(vot::CommandBuffer& cmd) -> void;
    private:
        entt::registry& ecs;

        float iTime{1.f};
    public:
        ev::pVkSetupContext ct{};
        ev::pVkRenderContext rt{};
        vot::RHandle RHandle = nullptr;
    };

} // sc

#endif //VKCELSHADINGRENDERER_RENDERSUBMISSIONSYSTEM_H
