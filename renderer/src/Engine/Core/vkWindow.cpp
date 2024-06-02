//
// Created by lenovo on 5/21/2024.
//

#include "vkWindow.h"

namespace yic {


    vkWindow::vkWindow(const int &w, const int &h) : mWidth{w}, mHeight{h},
        mWindow([&, initGLFW = []() -> std::optional<std::string>{
            if (!glfwInit()){
                return "failed to initialize GLFW";
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            return std::nullopt;
        }()] {
            if (initGLFW){
                throw std::runtime_error(initGLFW.value());
            }
            return std::shared_ptr<GLFWwindow>(
                    glfwCreateWindow(mWidth, mHeight, mName.c_str(),nullptr, nullptr),
                                    [](GLFWwindow *w) {
                        glfwTerminate();
                        glfwDestroyWindow(w);
                    });
        }()) {

        EventBus::publish(EventTypes::WindowContext{.window = mWindow});

        EventBus::subscribeAuto([&](const EventTypes::WindowContext& size){
            printf("width:%d\n", size.w.value());
        });
    }

    bool vkWindow::run() {
        [&,
                inputHandler = [&]() {
                    return InputHandlers::get(GetWindow());
                }()
        ]() {
            try {
                while (!glfwWindowShouldClose(GetWindow())) {
                    glfwPollEvents();

                    inputHandler->withDraw();

                    callback(GetWindow());
                }
            } catch (const std::exception &e) {
                std::cerr << "Exception caught in run loop: " << e.what() << "\n";
                return false;
            }
            return true;
        }();

        return true;
    }



} // yic