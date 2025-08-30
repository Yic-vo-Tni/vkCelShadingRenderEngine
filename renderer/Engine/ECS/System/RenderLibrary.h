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

        rhi::GraphicsPipeline GP_Basic_Assimp;
        rhi::GraphicsPipeline GP_Basic_PMX;
        rhi::GraphicsPipeline GP_Post;
        rhi::GraphicsPipeline GP_Volumetric_Overcast_Clouds;
        rhi::GraphicsPipeline GP_Volumetric_Fog;
        rhi::GraphicsPipeline GP_ShadowMap_Assimp;
        rhi::GraphicsPipeline GP_ShadowMap_PMX;
        rhi::GraphicsPipeline GP_Dummy;
        rhi::RayTracingPipeline RP_Shadow;

        enum eGBuffer : uint8_t {
            eAlbedo = 0,
            ePosition = 1,
            eNormal = 2,
            eCount
        };

        vot::Image_sptr RT_GBuffer;
        vot::Image_sptr RT_Post;
        vot::Image_sptr RT_Volumetric_Clouds;
        vot::Image_sptr RT_Volumetric_Fog;
        vot::Image_sptr RT_ShadowMap;

        vot::Image_sptr RTX_RayTracing;

        //
        vot::Image_sptr T_blueNoise64;
        vot::Image_sptr T_fbmNoise;
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
