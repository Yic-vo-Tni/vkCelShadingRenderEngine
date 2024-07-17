//
// Created by lenovo on 7/17/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
#define VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H

#include "vkRenderProcess.h"

namespace yic {

    class RenderProcessManager{
    public:
        vkGet auto get = []{ return Singleton<RenderProcessManager>::get();};

        RenderProcessManager();
        ~RenderProcessManager();

        static auto RenderProcedure() -> std::vector<vk::CommandBuffer>;
    private:
        tbb::concurrent_vector<vk::CommandBuffer> cmds;
        tbb::concurrent_unordered_map<std::string, std::shared_ptr<vkRenderProcess>> mRenderProcess;
        // t
        std::shared_ptr<GraphicsPipeline> mPipeline;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESSMANAGER_H
