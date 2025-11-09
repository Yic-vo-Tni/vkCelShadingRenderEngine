//
// Created by lenovo on 9/21/2025.
//

#ifndef VKCELSHADINGRENDERER_ATMOSPHERE_H
#define VKCELSHADINGRENDERER_ATMOSPHERE_H

#include "Defines.h"
#include "RHI/Pipeline/GraphicsPipeline.h"
#include "RHI/Pipeline/ComputePipeline.h"

namespace sm {
    class Atmosphere {
    public:
        Atmosphere();
        ~Atmosphere() = default;

    private:
        auto createPreset() -> void;
        auto loadAssets() -> void;
        auto createPipeline() -> void;

        rhi::GraphicsPipeline GP_Clouds;
        rhi::GraphicsPipeline GP_FarSky;
        rhi::ComputePipeline CP_TransmittanceLUT;
        rhi::ComputePipeline CP_SkyViewLUT;

        CloudsParametersBuffer cloudsParameters{};
        PostProcessParamsBuffer postProcessParameters{};
    };
} // sm

#endif //VKCELSHADINGRENDERER_ATMOSPHERE_H