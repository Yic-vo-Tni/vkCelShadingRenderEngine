//
// Created by lenovo on 6/12/2025.
//

#ifndef VKCELSHADINGRENDERER_GLOBAL_H
#define VKCELSHADINGRENDERER_GLOBAL_H

#include "pch.h"
#include "base.h"

namespace GLOBAL{
    inline bool visibleZMO{true};
    inline ImGuizmo::OPERATION gizmoOperation{ImGuizmo::TRANSLATE};
    inline vot::string pickON{};
    inline float fps;
    inline entt::entity camera;
    inline entt::entity set0;

    inline bool showVolumetricClouds{false};
    inline bool showVolumetricFog{false};
}

#endif //VKCELSHADINGRENDERER_GLOBAL_H
