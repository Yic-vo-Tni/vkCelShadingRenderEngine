#include <utility>

//
// Created by lenovo on 7/2/2024.
//

#ifndef VKCELSHADINGRENDERER_WIDGET_H
#define VKCELSHADINGRENDERER_WIDGET_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

#include "Editor/ShaderEditor/ShaderEditor.h"

namespace ui {

    using namespace yic;

    class Widget {
    public:
        explicit Widget() {
            mLastFrameTime = std::chrono::steady_clock::now();
            ShaderEditor::get();
        };
        ~Widget() = default;

        void render();

        void show() { mVisible = true;}
        void hide() { mVisible = false;}

    private:
        auto vkRenderWindow() -> void;
        auto vkViewWindow() -> void;
        auto vkControlPanelWindow() -> void;
        auto vkConsoleWindow() -> void;

        auto calculateFPS() -> void;
    protected:
        std::chrono::steady_clock::time_point mLastFrameTime;
        std::chrono::steady_clock::time_point mLastFpsTime;
        float mFps = 0.f;
        float mFpsTimer = 0.f;
        bool mVisible{};
    };

} // ui



#endif //VKCELSHADINGRENDERER_WIDGET_H
