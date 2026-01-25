//
// Created by lenovo on 10/3/2024.
//

#ifndef VKCELSHADINGRENDERER_IMGUI_H
#define VKCELSHADINGRENDERER_IMGUI_H

#include "Widget/Widget.h"

namespace ui {

    class ImGuiLauncher  {
    public:
        ImGuiLauncher ();
        ~ImGuiLauncher();

        auto draw(vk::CommandBuffer& cmd) -> void;
    private:
        auto base() -> void;
    private:
        bool mShowDemo{true};
        GLFWwindow* mWindow{};
        vk::Extent2D mExtent{};
        vk::Extent2D* mCurrentExtent{};
        vk::DescriptorPool mDescriptorPool{};
        vot::vector<std::unique_ptr<Widget>> mWidgets{};

        std::atomic_bool bSwap = false;
        bool mFocusMainWindow = true;
        auto updateSwap() -> void;
        std::chrono::microseconds swapInterval = std::chrono::microseconds(8);
        std::chrono::steady_clock::time_point lastSwapTime;
    };

} // ui

#endif //VKCELSHADINGRENDERER_IMGUI_H
