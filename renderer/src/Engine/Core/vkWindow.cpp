//
// Created by lenovo on 5/21/2024.
//

#include "vkWindow.h"

namespace yic {

    vkWindow::vkWindow(const int &w, const int &h) : mWidth{w}, mHeight{h}, mWindow(createWindow()) {
        EventBus::update(et::vkRenderContext{
                std::make_pair(mWidth, mHeight), vk::Extent2D{(uint32_t)mWidth, (uint32_t)mHeight}, mWindow.get()
        }, et::vkRenderContext::id::mainRender);

        if_debug {
            EventBus::subscribeAuto([&](const et::vkRenderContext &vkRenderContext) {
                vkTrance("width: {0}, height: {1}", vkRenderContext.width_v(), vkRenderContext.height_v());
            }, et::vkRenderContext::id::mainRender);
        };
    }

    bool vkWindow::run() {

        try {
            while (!glfwWindowShouldClose(GetWindow())) {
                glfwPollEvents();

                InputHandlers::get(GetWindow())->withDraw();

                callback(GetWindow());
            }
        } catch (const std::exception &e) {
            std::cerr << "Exception caught in run loop: " << e.what() << "\n";
            return false;
        }
        return true;
    }

    auto vkWindow::createWindow() -> std::shared_ptr<GLFWwindow> {
        auto initGLFW = [] -> std::optional<std::string> {
            if (!glfwInit())
                return "failed to initialize GLFW";
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RELEASE, GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
            return std::nullopt;
        }();

        if (initGLFW)
            throw std::runtime_error(initGLFW.value());

        return {glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr), [](GLFWwindow *w) {
            glfwDestroyWindow(w);
            glfwTerminate();
        }};
    }




} // yic
























