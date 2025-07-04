//
// Created by lenovo on 5/16/2025.
//

#include "ImGuiHub.h"

namespace ui {



    auto ImGuiHub::to(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        tasks[widget].push(fn);
    }

    auto ImGuiHub::bind(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        pers[widget].emplace_back(fn);
    }

    auto ImGuiHub::exe(const vot::uiWidget &widget) -> void {
        auto it = pers.find(widget);

        if (it != pers.end()){
            for(auto& fnn : it->second){
                fnn();
            }
        }


        std::function<void()> fn;

        while (tasks[widget].try_pop(fn)){
            fn();
        }
    }

    auto ImGuiHub::collapsingHeader(const char *label, const std::function<void()> &fn, ImGuiTreeNodeFlags flags,
                                    float indent) -> void {
        if (ImGui::CollapsingHeader(label, flags)) {
            ImGui::Indent(indent);
            fn();
            ImGui::Unindent(indent);
        }
    }

} // ui