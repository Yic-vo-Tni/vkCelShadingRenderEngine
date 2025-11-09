//
// Created by lenovo on 6/12/2025.
//

#ifndef VKCELSHADINGRENDERER_GLOBAL_H
#define VKCELSHADINGRENDERER_GLOBAL_H

#include "pch.h"
#include "base.h"
#include "ecs_struct.h"

namespace GLOBAL{
    inline bool visibleZMO{true};
    inline ImGuizmo::OPERATION gizmoOperation{ImGuizmo::TRANSLATE};
    inline vot::string pickON{};
    inline float fps;
    //inline entt::entity camera;
    // inline entt::entity set0;
    // inline vot::EntityView camera;

    inline bool showVolumetricClouds{false};
    inline bool showVolumetricFog{false};

    inline std::pair<float, float> mousePick{-1.f, -1.f};
    inline bool playAllAnim{false};

    namespace entity {
        inline vot::EntityView<vot::bit::eEntityViewDefault> set0;
        inline vot::EntityView<vot::bit::eEntityViewDefault> camera;
    }
}

#endif //VKCELSHADINGRENDERER_GLOBAL_H
