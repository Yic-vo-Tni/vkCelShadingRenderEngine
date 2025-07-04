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
        auto callback(GLFWwindow* window) -> void;
    private:
        bool mShowDemo{true};
        GLFWwindow* mWindow{};
        vk::Extent2D mExtent{};
        vk::Extent2D* mCurrentExtent{};
        vk::DescriptorPool mDescriptorPool{};
        vot::vector<std::unique_ptr<Widget>> mWidgets{};

        // t
//        ev::glKeyInput keyInput[2];
//        ev::glMouseInput mouseInput[2];
//        ev::glCursorPosInput cursorPosInput[2];
//        ev::glScrollInput scrollInput[2];
//        oneapi::tbb::spin_rw_mutex keyInputMutex;
//        oneapi::tbb::spin_rw_mutex mouseInputMutex;
//        oneapi::tbb::spin_rw_mutex cursorPosInputMutex;
//        oneapi::tbb::spin_rw_mutex scrollInputMutex;
        std::atomic_bool bSwap = false;

        auto updateSwap() -> void;
        std::chrono::microseconds swapInterval = std::chrono::microseconds(8);
        std::chrono::steady_clock::time_point lastSwapTime;
    };

} // ui

#endif //VKCELSHADINGRENDERER_IMGUI_H
