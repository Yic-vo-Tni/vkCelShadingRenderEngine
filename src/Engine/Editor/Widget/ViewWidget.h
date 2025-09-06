//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_VIEWWIDGET_H
#define VKCELSHADINGRENDERER_VIEWWIDGET_H

#include "Widget.h"

namespace ui {

    class ViewWidget : public Widget{
    public:
        ViewWidget();
        ~ViewWidget() = default;

        void rec() override;
    private:
    };

} // ui

#endif //VKCELSHADINGRENDERER_VIEWWIDGET_H
