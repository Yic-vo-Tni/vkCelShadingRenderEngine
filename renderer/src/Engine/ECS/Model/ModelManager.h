//
// Created by lenovo on 7/22/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELMANAGER_H
#define VKCELSHADINGRENDERER_MODELMANAGER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/RHI/RenderGroup.h"
#include "Engine/RHI/RTBuilder.h"
#include "Engine/RHI/RenderProcess.h"

namespace sc {

    class ModelManager {
    public:
        explicit ModelManager(const flecs::world* ecs);
        ~ModelManager();

    private:
        auto rebuild() -> void;
        auto subscribeModel() -> void;
        auto render() -> void;
    private:
        const flecs::world *ecs;
        vkImg_sptr mRenderTargetOffImg;
        vk::Extent2D mExtent{2560, 1440};
        oneapi::tbb::spin_rw_mutex mSubscribeModelMutex;
        flecs::query<Model> mModelQuery;
        yic::RenderProcess* mRenderHandle{};
        std::shared_ptr<yic::RenderGroupGraphics> mRenderGroupGraphics;


        ///t
        //std::unique_ptr<yic::RTBuilder> mRTBuilder;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MODELMANAGER_H
