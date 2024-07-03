//
// Created by lenovo on 7/2/2024.
//

#include "Widget.h"

namespace ui {

    void Widget::render() {
        vkRenderWindow();
        vkViewWindow();
        vkControlPanelWindow();
        vkConsoleWindow();
    }

    auto Widget::vkRenderWindow() -> void {
        calculateFPS();

        ImGui::Begin("Render");
        ImGui::Text("FPS: %.1f", mFps);

        auto viewportSize = ImGui::GetContentRegionAvail();

        ImGui::End();
    }

    auto Widget::vkViewWindow() -> void {
        ImGui::Begin("View");

        ImGui::End();
    }

    auto Widget::vkControlPanelWindow() -> void {
        ImGui::Begin("Panel");

        ImGui::End();
    }

    auto Widget::vkConsoleWindow() -> void {
        ImGui::Begin("Console");

        ImGui::End();
    }


    auto Widget::calculateFPS() -> void {
        auto currentFrameTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> deltaTime = currentFrameTime - mLastFrameTime;
        mLastFrameTime = currentFrameTime;

        mFpsTimer += deltaTime.count();

        if (mFpsTimer >= 1.f){
            mFps = 1.f / deltaTime.count();
            mFpsTimer = 0.f;
        }
    }



} // ui