//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_WIDGETMANAGER_H
#define VKCELSHADINGRENDERER_WIDGETMANAGER_H

#include "Widget.h"

#include "SubWindow/RenderWindow.h"

namespace ui {

    class WidgetManager {

    public:
        explicit WidgetManager() {

            mWidgets.emplace_back(std::make_shared<RenderWindow>());

        };

        ~WidgetManager() = default;

        void render();

        void show() { mVisible = true; }
        void hide() { mVisible = false; }

    private:
        auto vkViewWindow() -> void;
        auto vkControlPanelWindow() -> void;
        auto vkConsoleWindow() -> void;
        auto shaderFileWindow() -> void;

    protected:
        bool mVisible{};
        std::vector<std::shared_ptr<Widget>> mWidgets;

    };

} // ui

#endif //VKCELSHADINGRENDERER_WIDGETMANAGER_H
