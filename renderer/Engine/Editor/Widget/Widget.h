#include <utility>

//
// Created by lenovo on 10/5/2024.
//

#ifndef VKCELSHADINGRENDERER_WIDGET_H
#define VKCELSHADINGRENDERER_WIDGET_H


namespace ui {

    class Widget {
    public:
        Widget(vot::string name) : mWindowName(std::move(name)){}

        virtual auto rec() -> void = 0;

        auto getName() -> vot::string{ return mWindowName; };
    private:
        vot::string mWindowName;
    };

} // ui

#endif //VKCELSHADINGRENDERER_WIDGET_H
