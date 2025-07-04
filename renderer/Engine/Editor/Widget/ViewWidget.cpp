//
// Created by lenovo on 10/5/2024.
//

#include "ViewWidget.h"
#include "Editor/ImGuiHub.h"

namespace ui {
    ViewWidget::ViewWidget() : Widget("View"){

    }

    void ViewWidget::rec() {
        yic::imguiHub->exe(vot::uiWidget::eViewWidget);
    }
} // ui