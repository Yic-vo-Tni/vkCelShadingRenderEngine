//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_WIDGET_H
#define VKCELSHADINGRENDERER_WIDGET_H

#include <utility>

#include "Editor/ImGuiHub.h"

namespace ui {

    class Widget {
    public:
        virtual ~Widget() = default;

        explicit Widget(vot::string name) : mWindowName(std::move(name)){}

        virtual auto rec() -> void = 0;

        auto getName() -> vot::string{ return mWindowName; };
    private:
        vot::string mWindowName;
    };

    template<vot::uiWidget WIDGET_ENUM, const char* WidgetName>
    class GenericWidget final : public Widget {
        public:
        explicit GenericWidget() : Widget(WidgetName) {}

        auto rec() -> void override { yic::imguiHub->exe(WIDGET_ENUM); };
    };

    namespace window {
        constexpr char NameView[] = "View";
        constexpr char NameConsole[] = "Console";
        constexpr char NameRender[] = "Render";
        constexpr char NamePanel[] = "Panel";
        constexpr char NameNode[] = "NodeGraph";

        using view = GenericWidget<vot::uiWidget::eViewWidget, NameView>;
        using console = GenericWidget<vot::uiWidget::eConsoleWidget, NameConsole>;
        using render = GenericWidget<vot::uiWidget::eRenderWidget, NameRender>;
        using panel = GenericWidget<vot::uiWidget::ePanelWidget, NamePanel>;
        using nodeGraph = GenericWidget<vot::uiWidget::eNodeWidget, NameNode>;
    }
} // ui

#endif //VKCELSHADINGRENDERER_WIDGET_H
