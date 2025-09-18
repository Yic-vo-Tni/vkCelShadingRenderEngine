//
// Created by lenovo on 9/26/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_STRUCT_H
#define VKCELSHADINGRENDERER_ECS_STRUCT_H

#include "pch.h"
#include "spdlog.h"

namespace vot::inline sc{

struct DirectionLightComponent {
    std::string name;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity = 1.0f;
};

}


namespace vot {

    struct EntityView {
        EntityView() = default;

        explicit EntityView(const entt::entity& entity) {
            this->entity = entity;
        }

        static auto Create() -> EntityView {
            return EntityView{registry->create()};
        }

        template<typename C, typename... Args>
        EntityView& emplace(Args&&...args) {
            assert(entity != entt::null && "You must create the entity first!");
            registry->emplace<C>(entity, std::forward<Args>(args)...);
            return *this;
        }

        auto make() -> EntityView& {
            entity = registry->create();
            return *this;
        }

        template<typename C, typename... Args>
        auto makeVa(Args &&... args) -> C& {
            entity = registry->create();
            registry->emplace<C>(entity, std::forward<Args>(args)...);
            return registry->get<C>(entity);
        }

        template<typename C>
        auto va() ->C& { return registry->get<C>(entity); }

        template<typename C>
        auto va() const -> const C& { return registry->get<C>(entity); }

        template<typename C>
        [[nodiscard]] auto has() const -> bool { return registry->any_of<C>(entity); }

        auto destroy() const -> void { registry->destroy(entity); }

        static auto Init(entt::registry& entt){ registry = &entt; }
    private:
        entt::entity entity{entt::null};
        static inline entt::registry* registry{nullptr};
    };

}



#endif //VKCELSHADINGRENDERER_ECS_STRUCT_H
