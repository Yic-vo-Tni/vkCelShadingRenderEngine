//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_WINDOW_H
#define VKCELSHADINGRENDERER_WINDOW_H

namespace yic {

    class Window {
    public:
        explicit Window(vot::string name);
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
        auto setWindowIcon(GLFWwindow* window, const char* filename) const -> void;
        auto compWindowSize() -> void;
    private:
        int mWidth{}, mHeight{};
        vot::string mName{};
        GLFWwindow* mWindow{nullptr};

        bool firstClick = true;
        bool isRightMouseDown = false;
        double xLast{}, yLast{};

        std::atomic<bool> closeRequested{false};
        std::atomic<bool> closeRender{false};
    };

} // yic

#endif //VKCELSHADINGRENDERER_WINDOW_H
