//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERWIDGET_H
#define VKCELSHADINGRENDERER_RENDERWIDGET_H

#include "Widget.h"

namespace ui {

    class RenderWidget : public Widget{
    public:
        RenderWidget();
        ~RenderWidget() = default;

        void rec() override;

    private:
    };

} // ui

#endif //VKCELSHADINGRENDERER_RENDERWIDGET_H
