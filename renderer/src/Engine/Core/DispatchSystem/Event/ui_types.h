
//
// Created by lenovo on 7/2/2024.
//

#ifndef VKCELSHADINGRENDERER_UI_TYPES_H
#define VKCELSHADINGRENDERER_UI_TYPES_H


namespace et{

    struct uiWidgetContext{
        HANA(uiWidgetContext,
             (opt<ImVec2>, viewportSize));

        RETURN_VALUE(viewportSize);
        CHECK_MEMBER_EXISTS(viewportSize);
        RETURN_CUSTOM_VALUE(viewport_width, viewportSize, viewportSize->x);
        RETURN_CUSTOM_VALUE(viewport_height, viewportSize, viewportSize->y);
    };


}

#endif //VKCELSHADINGRENDERER_UI_TYPES_H
