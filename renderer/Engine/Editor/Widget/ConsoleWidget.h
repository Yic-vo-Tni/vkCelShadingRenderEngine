//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_CONSOLEWIDGET_H
#define VKCELSHADINGRENDERER_CONSOLEWIDGET_H

#include "Widget.h"

namespace ui {

    class ConsoleWidget : public Widget {
    public:
        ConsoleWidget();
        ~ConsoleWidget() = default;

        void rec() override;

    private:

    };

} // ui

#endif //VKCELSHADINGRENDERER_CONSOLEWIDGET_H
