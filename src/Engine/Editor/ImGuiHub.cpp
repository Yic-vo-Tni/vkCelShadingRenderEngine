//
// Created by lenovo on 5/16/2025.
//

#include "ImGuiHub.h"

namespace ui {



    auto ImGuiHub::to(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        slots[widget].tasks.push(fn);
    }

    auto ImGuiHub::to_fixed(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        slots[widget].fixed.emplace_back(fn);
    }

    auto ImGuiHub::bind(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        slots[widget].bind = fn;
    }

    auto ImGuiHub::exe(const vot::uiWidget &widget) -> void {
        auto& slot = slots[widget];

        if (slot.bind)
            slot.bind();
        for (auto& fn : slot.fixed)
            fn();
        std::function<void()> task;
        while (slot.tasks.try_pop(task))
            task();
    }

    auto ImGuiHub::collapsingHeader(const char *label, const std::function<void()> &fn, ImGuiTreeNodeFlags flags,
                                    float indent) -> void {
        if (ImGui::CollapsingHeader(label, flags)) {
            ImGui::Indent(indent);
            fn();
            ImGui::Unindent(indent);
        }
    }

    auto ImGuiHub::button(const char *label, const std::function<void()> &fn) -> void {
        if (ImGui::Button(label)) {
            fn();
        }
    }
} // ui