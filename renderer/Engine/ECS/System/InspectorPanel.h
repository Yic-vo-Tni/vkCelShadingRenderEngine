//
// Created by lenovo on 6/8/2025.
//

#ifndef VKCELSHADINGRENDERER_INSPECTORPANEL_H
#define VKCELSHADINGRENDERER_INSPECTORPANEL_H

namespace sc {

    class InspectorPanel {
    public:
        //explicit InspectorPanel(flecs::world& world);
        explicit InspectorPanel(entt::registry& registry);
        ~InspectorPanel() = default;

        auto frame() -> void;
    private:
//        flecs::world& ecs;
        entt::registry& ecs;

        float animationTime = 0.f;
        glm::mat4 modelMatrix{1.f};
    };

} // sc



#endif //VKCELSHADINGRENDERER_INSPECTORPANEL_H
