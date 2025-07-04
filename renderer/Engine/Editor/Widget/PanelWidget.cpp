//
// Created by lenovo on 10/5/2024.
//

#include "PanelWidget.h"
#include "Editor/ImGuiHub.h"

namespace ui {
    PanelWidget::PanelWidget() : Widget("Panel") {

    }

    void PanelWidget::rec() {
        yic::imguiHub->exe(vot::uiWidget::ePanelWidget);
    }
} // ui