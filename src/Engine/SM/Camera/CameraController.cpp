//
// Created by lenovo on 1/24/2026.
//

#include "CameraController.h"

namespace vot::sm {
    auto CameraController::onMouseMove(const float &x, const float &y) -> void {
        if (firstMouse) {
            lastX = x;
            lastY = y;
            firstMouse = false;
        }

        auto dx = x - lastX;
        auto dy = lastY - y;
        lastX = x;
        lastY = y;

        dx *= mouseSensitivity;
        dy *= mouseSensitivity;

        state.yaw += dx;
        state.pitch += dy;

        state.pitch = glm::clamp(state.pitch, -89.f, 89.f);
    }

    auto CameraController::onScroll(const float &offset) -> void {
        state.fov -= offset;
        state.fov = glm::clamp(state.fov, 20.f, 90.f);
    }

    auto CameraController::calcForward(const float &yaw, const float &pitch) -> glm::vec3 {
        glm::vec3 f;
        f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        f.y = sin(glm::radians(pitch));
        f.z = sin(glm::radians(yaw));
        return f;
    }

    auto CameraController::moveForward(const float &dt) -> void {
        const auto forward = calcForward(state.yaw, state.pitch);
        state.position += forward * moveSpeed * dt;
    }

    auto CameraController::moveBackward(const float &dt) -> void {
        const auto backward = calcForward(state.yaw, state.pitch);
        state.position -= backward * moveSpeed * dt;
    }

    auto CameraController::moveLeft(const float &dt) -> void {
        const auto forward = calcForward(state.yaw, state.pitch);
        const auto right = glm::normalize(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f)));
        state.position -= right * moveSpeed * dt;
    }

    auto CameraController::moveRight(const float &dt) -> void {
        const auto forward = calcForward(state.yaw, state.pitch);
        const auto right = glm::normalize(glm::cross(forward, glm::vec3(0.f, 1.f, 0.f)));
        state.position += right * moveSpeed * dt;
    }
}
