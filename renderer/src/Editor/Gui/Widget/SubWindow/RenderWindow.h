//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERWINDOW_H
#define VKCELSHADINGRENDERER_RENDERWINDOW_H

#include "Editor/Gui/Widget/Widget.h"

namespace ui {

    class RenderWindow : public Widget{
    public:
        RenderWindow() : Widget("Render"){ mLastFrameTime = std::chrono::steady_clock::now();};

        void record() override;
        auto calculateFPS() -> void;

    private:
        ImVec2 mAvailable{0, 0};
    private:
        float mFps = 0.f;
        float mFpsTimer = 0.f;
        std::chrono::steady_clock::time_point mLastFpsTime;
        std::chrono::steady_clock::time_point mLastFrameTime;

    };

} // ui

#endif //VKCELSHADINGRENDERER_RENDERWINDOW_H
