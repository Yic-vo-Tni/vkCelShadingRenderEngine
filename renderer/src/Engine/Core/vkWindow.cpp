//
// Created by lenovo on 5/21/2024.
//

#include "vkWindow.h"

namespace yic {

    vkWindow::vkWindow(const int &w, const int &h) : mWidth{w}, mHeight{h}, mWindow(createWindow()) {
        mg::SystemHub.setEvent(ev::oWindowSizeChange{
                .size = ImVec2((float) mWidth, (float) mHeight),
                .extent = vk::Extent2D{(uint32_t) mWidth, (uint32_t) mHeight},
        });

        mg::SystemHub.sto(ev::hVkRenderContext{
                .window = mWindow.get()
        }, toolkit::enum_name(RenderPhase::ePrimary));

        mg::SystemHub.subscribe([&](const ev::oWindowSizeChange &change) {
            vkTrance("width: {0}, height: {1} ", change.extent->width, change.extent->height);
        });

        mTimePerFrame = sf::seconds(1.f / 120.f);
    }

    bool vkWindow::run() {

        try {
            while (!glfwWindowShouldClose(GetWindow())) {
                get()->beginFrame();

                InputHandlers::withDraw();
                glfwPollEvents();

                get()->endFrame();
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
            //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
            return std::nullopt;
        }();

        if (initGLFW)
            throw std::runtime_error(initGLFW.value());

        return {glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr), [](GLFWwindow *w) {
            glfwDestroyWindow(w);
            glfwTerminate();
        }};
    }

    auto vkWindow::beginFrame() -> void {
        mStart = mClock.getElapsedTime();
    }

    auto vkWindow::endFrame() -> void {
        mFrameTime = mClock.getElapsedTime() - mStart;

        if (mFrameTime < mTimePerFrame)
            sf::sleep(mTimePerFrame - mFrameTime);
    }


} // yic
























