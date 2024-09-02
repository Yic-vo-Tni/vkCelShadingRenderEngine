//
// Created by lenovo on 8/30/2024.
//

#ifndef VKCELSHADINGRENDERER_SETTING_TYPES_H
#define VKCELSHADINGRENDERER_SETTING_TYPES_H


namespace et{

    struct ResolutionRatio{
        HANA_OPT(ResolutionRatio,
                 (vk::Extent2D, extent));

        RETURN_VALUE(extent);
        RETURN_CUSTOM_VALUE(width, extent, extent->width);
        RETURN_CUSTOM_VALUE(height, extent, extent->height);
    };


}

#endif //VKCELSHADINGRENDERER_SETTING_TYPES_H
