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
        vkConsoleWindow();
    }


    auto WidgetManager::vkViewWindow() -> void {
        ImGui::Begin("View");

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