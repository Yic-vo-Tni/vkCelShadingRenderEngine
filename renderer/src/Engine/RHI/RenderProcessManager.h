//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
#define VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H

#include "Engine/RHI/RenderProcess.h"

#include "Engine/RHI/RenderGroup.h"
#include "Engine/ECS/ECSManager.h"


namespace yic {

    class RenderProcessManager{
    public:
        vkGet auto get = []{ return Singleton<RenderProcessManager>::get();};

        RenderProcessManager();
        ~RenderProcessManager();

        DEFINE_STATIC_ACCESSOR(prepare);
        DEFINE_STATIC_ACCESSOR(clear);
        DEFINE_STATIC_ACCESSOR(RenderProcedure);

    private:
        auto RenderProcedure_impl() -> std::vector<vk::CommandBuffer>;
        auto prepare_impl() -> void;
        auto drawBuild() -> void;
        auto clear_impl() -> void;
    private:
        std::unique_ptr<sc::ECSManager> mEcsManager;
        tbb::concurrent_vector<vk::CommandBuffer> cmds;
        tbb::concurrent_unordered_map<std::string, std::unique_ptr<RenderProcess>> mRenderProcess;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
