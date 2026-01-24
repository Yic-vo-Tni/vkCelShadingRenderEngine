//
// Created by lenovo on 1/24/2026.
//

#ifndef VKCELSHADINGRENDERER_CAMERAVIEW_H
#define VKCELSHADINGRENDERER_CAMERAVIEW_H
#include "CameraController.h"

namespace vot::sm {

    struct CameraView {
        glm::mat4 view{1.f};
        glm::mat4 proj{1.f};
        glm::mat4 vp{1.f};
        glm::mat4 invView{1.f};
        glm::mat4 invProj{1.f};

        glm::vec3 forward;
        glm::vec3 right;
        glm::vec3 up;
    };

    struct CameraFrustum {
        glm::vec3 cornersWS[8];
    };

    struct CameraUBO {
        glm::mat4 view{1.f};
        glm::mat4 proj{1.f};
        glm::mat4 vp{1.f};

        glm::mat4 invView{1.f};
        glm::mat4 invProj{1.f};

        glm::vec4 pos3_pad;
        glm::vec4 forward3_pad;
    };

    class CameraFrame {
    public:
        auto update() -> void;

        auto vaCameraView() const { return view; }
        auto vaCameraFrustum() const { return frustum; }
    private:
        auto buildCameraView(const CameraState& state) -> void;
        auto buildCameraFrustum() -> void;
    private:
        CameraView view = {};
        CameraFrustum frustum = {};
        CameraController controller_; //TODO: if has Input system, move to
    };
}

#endif //VKCELSHADINGRENDERER_CAMERAVIEW_H
