//
// Created by lenovo on 7/19/2024.
//

#include "RenderWindow.h"

namespace ui {


    void RenderWindow::record() {
        calculateFPS();

        auto index = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        ImVec2 available = ImGui::GetContentRegionAvail();

        ImGui::Text("Available width: %.0f, height: %.0f", available.x, available.y);
        if (mAvailable.x != available.x || mAvailable.y != available.y) {
            EventBus::publish(et::uiWidgetContext{
                    .viewportSize = available
            }, enum_name(RenderProcessPhases::ePrimary));

            mAvailable = available;
        }

        ImGuiDescriptorManager::drawImage(enum_name(RenderProcessPhases::ePrimary), ImVec2(available.x, available.y), index);
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
        ImGui::Text("FPS: %.1f", mFps);
    }


} // ui