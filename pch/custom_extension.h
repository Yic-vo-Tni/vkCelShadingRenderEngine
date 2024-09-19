//
// Created by lenovo on 9/18/2024.
//

#ifndef VKCELSHADINGRENDERER_CUSTOM_EXTENSION_H
#define VKCELSHADINGRENDERER_CUSTOM_EXTENSION_H

#include "pch.h"

namespace vot{
    struct aabb{
        glm::vec3 min;
        glm::vec3 max;

        aabb() : min(0.f), max(0.f) {}
        aabb(const glm::vec3& min, const glm::vec3& max) : min(min), max(max){}

        template<typename T>
        aabb(const T& min_x, const T& min_y, const T& min_z, const T& max_x, const T& max_y, const T& max_z) : min(min_x, min_y, min_z), max(max_x, max_y, max_z){}

        [[nodiscard]] bool contains(const glm::vec3& point) const {
            return (point.x >= min.x && point.x <= max.x) &&
                   (point.y >= min.y && point.y <= max.y) &&
                   (point.z >= min.z && point.z <= max.z);
        }

        [[nodiscard]] auto distance() const -> float {
            return glm::length(max - min);
        }

        [[nodiscard]] auto center() const -> glm::vec3 {
            return (max + min) / 2.f;
        }
    };
}

#endif //VKCELSHADINGRENDERER_CUSTOM_EXTENSION_H
