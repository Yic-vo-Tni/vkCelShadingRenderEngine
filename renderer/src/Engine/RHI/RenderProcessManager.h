//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
#define VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H

#include "Engine/RHI/RenderProcess.h"
#include "Engine/RHI/RenderGroup.h"

#include "Engine/ECS/Camera/Camera.h"
#include "Engine/ECS/Model/LoadModel.h"

namespace yic {

    class RenderProcessManager{
    public:
        vkGet auto get = []{ return Singleton<RenderProcessManager>::get();};

        RenderProcessManager();
        ~RenderProcessManager();

        static auto prepare() -> void;
        static auto RenderProcedure() -> std::vector<vk::CommandBuffer>;

        static auto clear() -> void{
            get()->mRenderProcess.clear();
            get()->mRenderGroup.reset();
        }
    private:
        tbb::concurrent_vector<vk::CommandBuffer> cmds;
        tbb::concurrent_unordered_map<std::string, std::shared_ptr<RenderProcess>> mRenderProcess;
        // t
        sc::Model model;
        std::vector<sc::Model> models;

        std::shared_ptr<RenderGroup> mRenderGroup;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
