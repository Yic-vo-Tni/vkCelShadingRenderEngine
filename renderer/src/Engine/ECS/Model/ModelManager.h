//
// Created by lenovo on 7/22/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELMANAGER_H
#define VKCELSHADINGRENDERER_MODELMANAGER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/ECS/Model/ModelLoader.h"
#include "Engine/RHI/RenderGroup.h"
#include "Engine/RHI/RTBuilder.h"
#include "Engine/RHI/RenderProcessT.h"

namespace sc {

    class ModelManager {
    public:
        explicit ModelManager(const flecs::world* ecs);
        ~ModelManager();

        auto prepare() -> void;

    private:
        auto subscribeModel() -> void;
    private:
        const flecs::world *ecs;
        std::atomic<vk::Extent2D> mExtent;
        oneapi::tbb::spin_rw_mutex mModelMutex;
        vkImg_sptr mRtStorageOffImg;
        std::shared_ptr<yic::RenderGroupGraphics> mRenderGroupGraphics;


        ///t
        std::unique_ptr<yic::RTBuilder> mRTBuilder;
        vkImg_sptr  mRenderOffImg;
        auto rebuild() -> void;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MODELMANAGER_H
