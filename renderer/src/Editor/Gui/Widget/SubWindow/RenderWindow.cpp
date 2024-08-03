//
// Created by lenovo on 7/19/2024.
//

#include "RenderWindow.h"

namespace ui {


    void RenderWindow::record() {
        calculateFPS();

        auto index = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        ImVec2 available = ImGui::GetContentRegionAvail();
        auto windowPos = ImGui::GetCursorScreenPos();

        if (mAvailable.x != available.x || mAvailable.y != available.y) {
            EventBus::publish(et::uiWidgetContext{
                    .viewportSize = available
            }, enum_name(RenderProcessPhases::ePrimary));

            mAvailable = available;
        }

        ImGuiDescriptorManager::drawImage(enum_name(RenderProcessPhases::ePrimary), ImVec2(available.x, available.y), index);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        char fps_text[32];
        sprintf(fps_text, "FPS: %.1f", mFps);
        drawList->AddText(ImVec2(windowPos.x + 10.f, windowPos.y + 10.f), IM_COL32_WHITE, fps_text);

        float textHeight = ImGui::GetTextLineHeightWithSpacing();
        char size_text[64];
        sprintf(size_text, "Available width: %.0f, height: %.0f", available.x, available.y);
        drawList->AddText(ImVec2(windowPos.x + 10.f, windowPos.y + textHeight), IM_COL32_WHITE, size_text);
    }


    auto RenderWindow::calculateFPS() -> void {
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