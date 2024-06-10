//
// Created by lenovo on 6/4/2024.
//

#ifndef VKCELSHADINGRENDERER_TASKTYPES_H
#define VKCELSHADINGRENDERER_TASKTYPES_H

namespace tt{

    enum class EngineFlow{
        eRhi, ePhysical, eGameLogical
    };

    enum class RebuildSwapchain{
        eSwapchainRebuild, eFrameBuffersRebuild
    };


}

#endif //VKCELSHADINGRENDERER_TASKTYPES_H
