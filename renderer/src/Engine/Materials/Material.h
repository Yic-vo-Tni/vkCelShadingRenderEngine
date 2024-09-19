//
// Created by lenovo on 9/14/2024.
//

#ifndef VKCELSHADINGRENDERER_MATERIAL_H
#define VKCELSHADINGRENDERER_MATERIAL_H

//#include "../pch/pch.h"

namespace mt{

    struct Material{
        Material() = default;

        Material(glm::vec3 rgb, float roughness, float metallic) : albedo(rgb, 1.f), roughness(roughness),
                                                                   metallic(metallic) {
        }

        Material(glm::vec4 albedo, float roughness, float metallic) : albedo(albedo), roughness(roughness),
                                                                      metallic(metallic) {
        }

        glm::vec4 albedo{};
        float roughness{};
        float metallic{};
        float specular{};
        float opacity{};
        glm::vec4 emissive{};
    };

}


#endif //VKCELSHADINGRENDERER_MATERIAL_H
