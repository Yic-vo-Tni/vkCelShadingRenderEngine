//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_WINDOW_H
#define VKCELSHADINGRENDERER_WINDOW_H

//#include "ShaderHotReload/ShaderEditor.h"

namespace yic {

    struct glT{
        ev::glKeyInput keyInput;
        inline static std::atomic<int> keyInputActive{0};
        ev::glMouseInput mouseInput;
        inline static std::atomic<int> mouseInputActive{0};
        ev::glScrollInput scrollInput;
        inline static std::atomic<int> scrollInputActive{0};
        ev::glCursorPosInput cursorPosInput;
        inline static std::atomic<int> cursorPosInputActive{0};
    };
    inline std::array<glT, 2> glTBuffers;

    class Window {
    public:
        Window(const int& w, const int& h, vot::string name);
        ~Window();

        auto loop(const std::function<void()>& fn) const -> bool;
        auto& shouldClose() { return closeRequested; }
        auto renderClosed() -> void { return closeRender.store(true, std::memory_order_relaxed); }
        auto onClose() -> void{
            closeRequested.store(true, std::memory_order_relaxed);
            glfwSetWindowShouldClose(mWindow, GLFW_FALSE);
        }
    private:
        auto controller() -> void;
        auto createWindow() -> GLFWwindow*;

    private:
        int mWidth{}, mHeight{};
        vot::string mName{};
        GLFWwindow* mWindow{nullptr};

        bool firstClick = true;
        bool isRightMouseDown = false;
        double xLast{}, yLast{};

        std::atomic<bool> closeRequested{false};
        std::atomic<bool> closeRender{false};

///
//        mutable ui::ShaderEditor shaderEditor;
    };

} // yic

#endif //VKCELSHADINGRENDERER_WINDOW_H
