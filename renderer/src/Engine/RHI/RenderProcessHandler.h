//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPROCESSHANDLER_H
#define VKCELSHADINGRENDERER_RENDERPROCESSHANDLER_H

#include "Engine/RHI/RenderProcess.h"
#include "Engine/RHI/RenderGroup.h"
#include "Engine/ECS/ECSManager.h"


namespace yic {

    class RenderProcessHandler{
    public:
        RenderProcessHandler();
        ~RenderProcessHandler();

        auto procedure() -> std::vector<vk::CommandBuffer>;
        auto prepare() -> void;
        auto clear() -> void;
    private:
        auto registerRenderProcess(RenderPhase phases) -> RenderProcess*;
    private:
        std::unique_ptr<sc::ECSManager> mEcsManager;

        vot::vector<std::unique_ptr<RenderProcess>> mRenderProcess;
        std::vector<vk::CommandBuffer> cmds;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESSHANDLER_H
