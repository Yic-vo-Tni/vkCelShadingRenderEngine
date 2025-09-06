//
// Created by lenovo on 6/8/2025.
//

#ifndef VKCELSHADINGRENDERER_INSPECTORPANEL_H
#define VKCELSHADINGRENDERER_INSPECTORPANEL_H

namespace sc {

    class InspectorPanel {
        template<typename T, typename FnHas, typename FnNot>
        void if_has(const entt::entity e, FnHas &&fnHas, FnNot &&fnNot) {
            if (ecs.all_of<T>(e)) fnHas();
            else fnNot();
        }
    public:
        explicit InspectorPanel(entt::registry& registry);
        ~InspectorPanel() = default;

        auto frame() -> void;

    private:
        auto drawAnimComboForMMD(const entt::entity& entity, vot::AnimationComponent& ac) -> void;
        auto drawAnimComboForGeneric(vot::AnimationComponent& ac) -> void;
        auto drawPlayButton(vot::BasicInfoComponent& info, vot::AnimationComponent& ac) -> void;
        auto drawGizmo() -> void;
        auto drawModelList() -> void;
        auto drawShaderFileList() -> void;
        auto mousePick() -> void;
    private:
        entt::registry& ecs;
    };

} // sc



#endif //VKCELSHADINGRENDERER_INSPECTORPANEL_H
