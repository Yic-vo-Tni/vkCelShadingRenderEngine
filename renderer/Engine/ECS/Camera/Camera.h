//
// Created by lenovo on 10/23/2024.
//

#ifndef VKCELSHADINGRENDERER_CAMERA_H
#define VKCELSHADINGRENDERER_CAMERA_H

#include "RHI/Allocator.h"

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
        VpMatrix mVpMatrix{};
        ImVec2 mSize{2560, 1440};

        float fov   =  45.0f;
        float sensitivity = 0.1f;
        friend Singleton<Camera>;
        glm::vec3 position;
        float dynamicSpeed = 1.f;
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

        vot::Buffer_sptr buf{};
    public:
        bool firstMouse = true;
        Camera() : position(0.f, 0.f, 25.f), orientation(glm::quat(1., 0., 0., 0.)) {
        }
        ~Camera(){
            clear();
        }
        [[nodiscard]] inline auto& getPosition()  { return position;}
        [[nodiscard]] inline auto& getCameraFront() const { return cameraFront;}
        [[nodiscard]] inline auto& getCameraUp() const { return cameraUp;}
        [[nodiscard]] inline auto& getDynamicSpeed() const { return dynamicSpeed;}
       // [[nodiscard]] inline auto& getVpMatrix() const { return mVp;}
        [[nodiscard]] inline auto& getView() const { return mView;}
        [[nodiscard]] inline auto& getProj() const { return mProj;}
        [[nodiscard]] inline auto& getVpMatrixBuf() const { return buf;}
        [[nodiscard]] inline auto vpBufferInfo() const { return buf->bufferInfo();}
        vot::DescriptorHandle DS;

        auto clear() -> void{
            buf.reset();
        }

        auto computeViewMatrix() -> void{
            mView = glm::translate(glm::mat4_cast(orientation), -position);
            //mViewInverse = glm::inverse(mView);
            mVpMatrix.vInverse = glm::inverse(mView);
        }
        auto computeProjMatrix() -> void{
            mProj = glm::perspective(fov, mSize.x / mSize.y, 0.1f, 500.f) * glm::scale(glm::mat4(1.f), glm::vec3(1.f, -1.f, 1.f));
            //mProjInverse = glm::inverse(mProj);
            mVpMatrix.pInverse = glm::inverse(mProj);
        }

        auto computeViewProjMatrix(){
            computeViewMatrix();
            computeProjMatrix();

            //mVp = mProj * mView;
            mVpMatrix.vp = mProj * mView;
            if (buf){
                buf->update(mVpMatrix);
            } else {
                //buf = yic::allocator->allocBuffer(sizeof(glm::mat4), &mVp, vk::BufferUsageFlagBits::eUniformBuffer, " camera");
                buf = yic::allocator->allocBuffer(sizeof(VpMatrix), &mVpMatrix, vk::BufferUsageFlagBits::eUniformBuffer, " camera");
            }

            return *this;
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


//inline static constexpr const char* eGlobalCamera = "global_camera";
//
//inline auto camera_comp(flecs::world& ecs){
//    return ecs.entity(eGlobalCamera).get<sc::Camera>();
//}
//inline entt::entity eGlobalCamera;

//#define camera_comp ecs.entity(sc::eGlobalCamera).get<sc::Camera>()

} // sc



#endif //VKCELSHADINGRENDERER_CAMERA_H
