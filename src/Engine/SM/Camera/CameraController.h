//
// Created by lenovo on 1/24/2026.
//

#ifndef VKCELSHADINGRENDERER_CAMERACONTROLLER_H
#define VKCELSHADINGRENDERER_CAMERACONTROLLER_H

namespace vot::sm {
    struct CameraState {
        glm::vec3 position{0.f, 0.f, 0.25f};
        float yaw = -90.f;
        float pitch = 0.f;

        float fov = 45.f;
        float aspect = 16.f / 9.f;
        float nearZ = 0.1f;
        float farZ = 500.f;
    };

    class CameraController {
    public:
        auto onMouseMove(const float& x, const float& y) -> void;
        auto onScroll(const float& offset) -> void;

        auto update() -> void;
        auto calcForward(const float& yaw, const float& pitch) -> glm::vec3;
        auto moveForward(const float& dt) -> void;
        auto moveBackward(const float& dt) -> void;
        auto moveLeft(const float& dt) -> void;
        auto moveRight(const float& dt) -> void;

        //
        auto vaCameraState() const { return state; }
        auto onFirstMouse() { return firstMouse = true; }
    private:
        float mouseSensitivity{0.1f};
        float moveSpeed{1.f};
        bool firstMouse{true};
        float lastX{0.f}, lastY{0.f};
        CameraState state{};
    };
}

#endif //VKCELSHADINGRENDERER_CAMERACONTROLLER_H
