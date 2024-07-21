//
// Created by lenovo on 7/19/2024.
//

#include "WidgetManager.h"

namespace ui {

    void WidgetManager::render() {

        for(auto& widget : mWidgets){
            ImGui::Begin(widget->getName().c_str());

            widget->record();

            ImGui::End();
        }

        vkViewWindow();
        vkControlPanelWindow();
        vkConsoleWindow();
    }


    auto WidgetManager::vkViewWindow() -> void {
        ImGui::Begin("View");

        ImGui::End();
    }

    auto WidgetManager::vkControlPanelWindow() -> void {
        ImGui::Begin("Panel");

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        if (ImGui::Selectable(" f_test.frag", false, ImGuiSelectableFlags_DontClosePopups)) {
            ShaderEditor::openShaderFile(R"(F:\Yicvot\vkCelShadingRenderer\renderer\assets\shaders\f_test.frag)");
        }
        if (ImGui::Selectable(" v_test.vert", false, ImGuiSelectableFlags_DontClosePopups)) {
            ShaderEditor::openShaderFile(R"(F:\Yicvot\vkCelShadingRenderer\renderer\assets\shaders\v_test.vert)");
        }

        ImGui::PopStyleColor(1);

        ImGui::End();
    }

    auto WidgetManager::vkConsoleWindow() -> void {
        ImGui::Begin("Console");

        ImGui::End();
    }

    auto WidgetManager::shaderFileWindow() -> void {
//        ImGui::Begin("ShaderFile");
//
//
//
//        ImGui::End();
    }


} // ui