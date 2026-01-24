//
// Created by lenovo on 1/24/2026.
//

#include "CameraFrame.h"

#include "Core/DispatchSystem/SystemHub.h"

namespace vot::sm {
    auto CameraFrame::update() -> void {
        const auto& state = controller_.vaCameraState();
        {
            auto fLock = yic::systemHub.vaL<ev::vFreeCameraController>();

            if (fLock->W) controller_.moveForward(GLOBAL::dt.load(std::memory_order_relaxed));
            if (fLock->S) controller_.moveBackward(GLOBAL::dt.load(std::memory_order_relaxed));
            if (fLock->A) controller_.moveLeft(GLOBAL::dt.load(std::memory_order_relaxed));
            if (fLock->D) controller_.moveRight(GLOBAL::dt.load(std::memory_order_relaxed));
            if (fLock->cursor) controller_.onMouseMove(fLock->xPos, fLock->yPos);
            if (fLock->scroll) controller_.onScroll(fLock->yOffset);
            if (fLock->firstM) controller_.onFirstMouse();

            fLock->W = false;
            fLock->S = false;
            fLock->A = false;
            fLock->D = false;
            fLock->cursor = false;
            fLock->scroll = false;
            fLock->firstM = false;
        }

        buildCameraView(state);
        buildCameraFrustum();
    }

    auto CameraFrame::buildCameraView(const CameraState& state) -> void {
        glm::vec3 forward{
            cos(glm::radians(state.yaw)) * cos(glm::radians(state.pitch)),
            sin(glm::radians(state.pitch)),
            sin(glm::radians(state.yaw)) * cos(glm::radians(state.pitch)),
        };

        forward = glm::normalize(forward);

        constexpr glm::vec3 worldUp{0.f, 1.f, 0.f};
        const glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
        const glm::vec3 up = glm::normalize(glm::cross(right, forward));

        view.forward = forward;
        view.right = right;
        view.up = up;

        view.view = glm::lookAt(state.position, state.position + forward, up);
        view.invView = glm::inverse(view.view);

        view.proj = glm::perspective(glm::radians(state.fov), state.aspect, state.nearZ, state.farZ);
        view.proj[1][1] *= -1.f;
        view.invProj = glm::inverse(view.proj);

        view.vp = view.proj * view.view;
    }

    auto CameraFrame::buildCameraFrustum() -> void {
        const glm::mat4 inVp = glm::inverse(view.vp);

        const glm::vec4 ndcCorners[8] = {
            {-1.f, -1.f, 0.f, 1.f}, {1.f, -1.f, 0.f, 1.f},
            {1.f, 1.f, 0.f, 1.f}, {-1.f, 1.f, 0.f, 1.f},
            {-1.f, -1.f, 1.f, 1.f}, {1.f, -1.f, 1.f, 1.f},
            {1.f, 1.f, 1.f, 1.f}, {-1.f, 1.f, 1.f, 1.f}
        };

        for (auto i = 0u; i < 8; ++i) {
            glm::vec4 ws = inVp * ndcCorners[i];
            frustum.cornersWS[i] = glm::vec3(ws) / ws.w;
        }
    }
}
