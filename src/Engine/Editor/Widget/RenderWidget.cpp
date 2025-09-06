//
// Created by lenovo on 10/5/2024.
//

#include "RenderWidget.h"
#include "RHI/Descriptor.h"
#include "Editor/ImGuiHub.h"

namespace ui {
    RenderWidget::RenderWidget() : Widget("Render"){

    }

    void RenderWidget::rec() {
        //yic::imguiImage->drawImage("Post RT Image");
        yic::imguiHub->exe(vot::uiWidget::eRenderWidget);
    }

} // ui