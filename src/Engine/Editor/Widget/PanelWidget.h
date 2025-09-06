//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_PANELWIDGET_H
#define VKCELSHADINGRENDERER_PANELWIDGET_H

#include "Widget.h"

namespace ui {

    class PanelWidget : public Widget{
    public:
        PanelWidget();
        ~PanelWidget() = default;

        void rec() override;

    private:
    };

} // ui

#endif //VKCELSHADINGRENDERER_PANELWIDGET_H
