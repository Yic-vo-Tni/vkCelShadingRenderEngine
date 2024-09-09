//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_CAMERA_H
#define VKCELSHADINGRENDERER_CAMERA_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/Allocator.h"

namespace sc {

    struct VpMatrix{
        glm::mat4 vp;
        glm::mat4 vInverse;
        glm::mat4 pInverse;
    };

    class Camera {
        float lastX =  1200.0f / 2.0;
        float lastY =  800.0 / 2.0;
        float Yaw_t = -90.0f;
        float pitch_t = 0.0f;

        glm::quat orientation;
        glm::mat4 mView{1.f};
        glm::mat4 mProj{1.f};
        glm::mat4 mViewInverse{1.f};
        glm::mat4 mProjInverse{1.f};
        glm::mat4 mVp{};
        ImVec2 mSize{2560, 1440};

        float fov   =  45.0f;
        float sensitivity = 0.1f;
        friend Singleton<Camera>;
        glm::vec3 position;
        float dynamicSpeed = 1.f;
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//        vk::Extent2D extent;
        vkBuf_sptr buf{};
    public:
        bool firstMouse = true;
        Camera() : position(0.f, 0.f, 25.f), orientation(glm::quat(1., 0., 0., 0.)) {
        }
        [[nodiscard]] inline auto& getPosition()  { return position;}
        [[nodiscard]] inline auto& getCameraFront() const { return cameraFront;}
        [[nodiscard]] inline auto& getCameraUp() const { return cameraUp;}
        [[nodiscard]] inline auto& getDynamicSpeed() const { return dynamicSpeed;}
        [[nodiscard]] inline auto& getVpMatrix() const { return mVp;}
        [[nodiscard]] inline auto& getVpMatrixBuf() const { return buf;}

        auto initBuf() -> void{
            buf = yic::Allocator::allocBuf(sizeof(VpMatrix), vk::BufferUsageFlagBits::eUniformBuffer, yic::Allocator::MemoryUsage::eCpuToGpu, "global_camera");
            //extent = yic::EventBus::val<et::ResolutionRatio>().extent_v();
        }
        auto clear() -> void{
            buf.reset();
        }

        auto computeViewMatrix() -> void{
            mView = glm::translate(glm::mat4_cast(orientation), -position);
            mViewInverse = glm::inverse(mView);
        }
        auto computeProjMatrix() -> void{
//            if (yic::EventBus::val<et::uiWidgetContext>(enum_name(RenderPhase::ePrimary)).viewportSize_exists()){
//                mSize = yic::EventBus::val<et::uiWidgetContext>(enum_name(RenderPhase::ePrimary)).viewportSize_v();
//
//                //  bad = =
//            }

            mProj = glm::perspective(fov, mSize.x / mSize.y, 0.1f, 500.f) *
                    glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f));
            mProjInverse = glm::inverse(mProj);
        }

        auto computeViewProjMatrix() -> void{
            computeViewMatrix();
            computeProjMatrix();
            mVp = mProj * mView;
            buf->updateBuf(VpMatrix{
                .vp = mVp,
                .vInverse = mViewInverse,
                .pInverse = mProjInverse
            });
        }

        auto rotateCamera(float angle, float axis_x, float axis_y, float axis_z) -> void{
            glm::quat newRotate = glm::angleAxis(glm::radians(angle), glm::vec3 (axis_x, axis_y, axis_z));
            orientation = newRotate * orientation;
        }

        void mouseCallback(double xPos_d, double yPos_d) {
            auto xPos = static_cast<float>(xPos_d);
            auto yPos = static_cast<float>(yPos_d);
            if (firstMouse){
                lastX = xPos;
                lastY = yPos;
                firstMouse = false;
            }
            float xOffset = xPos - lastX;
            float yOffset = lastY - yPos;
            lastX = xPos;
            lastY = yPos;

            xOffset *= sensitivity;
            yOffset *= sensitivity;

            Yaw_t += xOffset;
            pitch_t += yOffset;
            rotate(xOffset, yOffset);
            updateCameraFront(Yaw_t, pitch_t);
        }

        void scrollCallback(double xOffset, double yOffset) {
            if (yOffset > 0.0) {
                dynamicSpeed *= 1.2f;
            } else if (yOffset < 0.0) {
                dynamicSpeed *= 0.8f;
            }
        }

    private:
        void rotate(float yaw, float pitch) {
            glm::vec3 localUp = glm::rotate(orientation, glm::vec3(0.f, 1.f, 0.f));
            glm::quat mPitch = glm::angleAxis(glm::radians(-pitch), glm::vec3(1.f, 0.f, 0.f));
            glm::quat mYaw = glm::angleAxis(glm::radians(yaw), localUp);
            orientation = mPitch * mYaw * orientation;
            orientation = glm::normalize(orientation);
        }

        void updateCameraFront(float yaw, float pitch) {
            glm::vec3 front;
            front.x = float(cos(glm::radians(yaw)) * cos(glm::radians(pitch)));
            front.y = float(sin(glm::radians(pitch)));
            front.z = float(sin(glm::radians(yaw)) * cos(glm::radians(pitch)));
            cameraFront = glm::normalize(front);
        }

    };

    inline Camera globalCamera{};



} // sc

#endif //VKCELSHADINGRENDERER_CAMERA_H
