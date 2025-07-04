//
// Created by lenovo on 10/5/2024.
//

#include "ConsoleWidget.h"
#include "Editor/ImGuiHub.h"

namespace ui {
    ConsoleWidget::ConsoleWidget() : Widget("Console"){

    }

    void ConsoleWidget::rec() {
        yic::imguiHub->exe(vot::uiWidget::eConsoleWidget);
    }

} // ui