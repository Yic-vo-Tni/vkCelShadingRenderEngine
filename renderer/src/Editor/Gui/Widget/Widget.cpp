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

        auto index = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        ImVec2 available = ImGui::GetContentRegionAvail();
        float aspectRatio = 1920.0f / 1080.0f;
        float imageWidth, imageHeight;

        if (available.x / available.y > aspectRatio) {
            imageHeight = available.y;
            imageWidth = imageHeight * aspectRatio;
        } else {
            imageWidth = available.x;
            imageHeight = imageWidth / aspectRatio;
        }
        ImGuiDescriptorManager::drawImage(enum_name(RenderProcessPhases::ePrimary), ImVec2(imageWidth, imageHeight), index);

        if (ImGui::Selectable("  Click Me")) {
            ShaderEditor::openShaderFile(R"(F:\Yicvot\vkCelShadingRenderer\renderer\assets\shaders\v_test.vert)");
        }

        ImGui::End();
    }

    auto Widget::vkViewWindow() -> void {
        ImGui::Begin("View");

//        // First part of the window
//        ImGui::BeginChild("Child1", ImVec2(20, 20), true);
//        ImGui::Text("First Part of the Window");
//        // You can put anything here
//        ImGui::EndChild();
//
//        // Splitter code here
//        ImGui::InvisibleButton("splitter", ImVec2(-1, 8.0));
//
//        // Second part of the window
//        ImGui::BeginChild("Child2", ImVec2(0, 20), true);
//        ImGui::Text("Second Part of the Window");
//        // You can put anything here
//        ImGui::EndChild();

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