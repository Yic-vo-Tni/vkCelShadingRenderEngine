//
// Created by lenovo on 9/26/2024.
//

#ifndef VKCELSHADINGRENDERER_ECS_STRUCT_H
#define VKCELSHADINGRENDERER_ECS_STRUCT_H

#include "pch.h"

namespace vot::inline sc{

struct DirectionLightComponent {
    std::string name;
    glm::vec3 direction;
    glm::vec3 color;
    float intensity = 1.0f;
};

}



#endif //VKCELSHADINGRENDERER_ECS_STRUCT_H
