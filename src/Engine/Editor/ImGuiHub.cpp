//
// Created by lenovo on 5/16/2025.
//

#include "ImGuiHub.h"

namespace ui {



    auto ImGuiHub::to(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        // tasks[widget].push(fn);
        slots[widget].tasks.push(std::move(fn));
    }

    auto ImGuiHub::to_fixed(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        // pers[widget].emplace_back(fn);
        slots[widget].fixed.emplace_back(std::move(fn));
    }

    auto ImGuiHub::bind(const vot::uiWidget &widget, const std::function<void()> &fn) -> void {
        //binds[widget] = fn;
        slots[widget].bind = std::move(fn);
    }

    auto ImGuiHub::exe(const vot::uiWidget &widget) -> void {
        // auto it_ = binds.find(widget);
        // if (it_ != binds.end()) {
        //     it_->second();
        // }
        //
        // auto it = pers.find(widget);
        //
        // if (it != pers.end()){
        //     for(auto& fnn : it->second){
        //         fnn();
        //     }
        // }
        //
        //
        // std::function<void()> fn;
        //
        // while (tasks[widget].try_pop(fn)){
        //     fn();
        // }

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