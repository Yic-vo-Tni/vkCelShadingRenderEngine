//
// Created by lenovo on 7/22/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELMANAGER_H
#define VKCELSHADINGRENDERER_MODELMANAGER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/ECS/Model/ModelLoader.h"
#include "Engine/RHI/RenderGroup.h"
//#include "Engine/ECS/Model/PmxLoader.h"
#include "Engine/RHI/RTBuilder.h"

namespace sc {

    struct PushConstantRay{
        glm::vec4 clearColor;
        glm::vec3 lightPosition;
        float lightIntensity;
        int lightType;
    };

    class ModelManager {
    public:
        explicit ModelManager(const flecs::world* ecs);
        ~ModelManager();

        auto Render(const vk::CommandBuffer& cmd) -> void;
        auto renderRt(const vk::CommandBuffer& cmd) -> void;
        auto subscribeModel() -> void;
    private:
        const flecs::world *ecs;
        std::atomic<vk::Extent2D> mExtent;
        oneapi::tbb::spin_rw_mutex mModelMutex;
        vkImg_sptr mRtStorageOffImg;
        std::shared_ptr<yic::RenderGroupGraphics> mRenderGroupGraphics;
        std::shared_ptr<yic::RenderGroupRayTracing> mRenderGroupRayTracing;


        ///t
        PushConstantRay p{};

//        vkAccel_sptr tlas;
        std::unique_ptr<yic::RTBuilder> mRTBuilder;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MODELMANAGER_H
