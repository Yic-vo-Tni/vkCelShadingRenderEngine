//
// Created by lenovo on 8/5/2024.
//

#ifndef VKCELSHADINGRENDERER_PANELWINDOW_H
#define VKCELSHADINGRENDERER_PANELWINDOW_H

#include "Editor/Gui/Widget/Widget.h"

#include "Engine/Core/FileOperator/FileOperation.h"

namespace ui {

    class PanelWindow : public Widget{
    public:
        PanelWindow() : Widget("Panel") {}

        void record() override;

        auto shaderFilePaths() -> void;
    private:
        std::vector<std::string> mShaderFilePaths;
    };

} // ui

#endif //VKCELSHADINGRENDERER_PANELWINDOW_H
