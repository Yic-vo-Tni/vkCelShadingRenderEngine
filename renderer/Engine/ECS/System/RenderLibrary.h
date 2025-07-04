//
// Created by lenovo on 5/20/2025.
//

#ifndef VKCELSHADINGRENDERER_RENDERLIBRARY_H
#define VKCELSHADINGRENDERER_RENDERLIBRARY_H

#include "RHI/Pipeline/GraphicsPipeline.h"
#include "RHI/Pipeline/RayTracingPipeline.h"
#include "RHI/Descriptor.h"

namespace sc {

    class RenderLibrary {
        using fnUniqueDescirptor = std::function<void()>;
    public:
        MAKE_SINGLETON(RenderLibrary);
        RenderLibrary();
        ~RenderLibrary();

        rhi::GraphicsPipeline GP_Basic;
        rhi::GraphicsPipeline GP_Basic_PMX;
        rhi::GraphicsPipeline GP_Post;
        rhi::GraphicsPipeline GP_Volumetric_Overcast_Clouds;
        rhi::RayTracingPipeline RP_Shadow;

        vot::Image_sptr RT_Main;
        vot::Image_sptr RT_Post;
        vot::Image_sptr RT_Volumetric_Overcast_Clouds;
        vot::Image_sptr RT_RayTracing;
    private:
        auto buildPipelines() -> void;
        auto buildRenderTarget() -> void;
        auto buildUniqueDSHandle() -> void;

    private:
        uint32_t frameImageCount{};
    };

} // sc

namespace yic{
    inline sc::RenderLibrary* renderLibrary;
}
#endif //VKCELSHADINGRENDERER_RENDERLIBRARY_H
