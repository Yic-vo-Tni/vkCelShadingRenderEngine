//
// Created by lenovo on 9/24/2024.
//

#include "Window.h"
#include "Core/DispatchSystem/SystemHub.h"

#include <utility>

namespace yic {

    inline auto framebufferSizeCallback = [](GLFWwindow *w, int width, int height) {

    };

    inline auto setKeyCallback = [](GLFWwindow *w, int key, int scancode, int action, int mods) {
        auto writeIndex = 1 - glT::keyInputActive.load(std::memory_order_acquire);
        glTBuffers[writeIndex].keyInput.key = key;
        glTBuffers[writeIndex].keyInput.scancode = scancode;
        glTBuffers[writeIndex].keyInput.action = action;
        glTBuffers[writeIndex].keyInput.mods = mods;
        glT::keyInputActive.store(writeIndex, std::memory_order_release);

        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            GLOBAL::visibleZMO = !GLOBAL::visibleZMO;
        }
//        yic::systemHub.enqueueEvent(ev::glKeyInput{key, action, scancode, mods});

    };

    inline auto setMouseButtonCallback = [](GLFWwindow *w, int button, int action, int mods) {
        auto writeIndex = 1 - glT::mouseInputActive.load(std::memory_order_acquire);
        glTBuffers[writeIndex].mouseInput.button = button;
        glTBuffers[writeIndex].mouseInput.action = action;
        glTBuffers[writeIndex].mouseInput.mods = mods;
        glT::mouseInputActive.store(writeIndex, std::memory_order_release);
    };

    inline auto setCursorPosCallback = [](GLFWwindow*w, double xpos, double ypos){
        auto writeIndex = 1 - glT::cursorPosInputActive.load(std::memory_order_acquire);
        glTBuffers[writeIndex].cursorPosInput.xpos = xpos;
        glTBuffers[writeIndex].cursorPosInput.ypos = ypos;
        glT::cursorPosInputActive.store(writeIndex, std::memory_order_release);
    };

    inline auto setScrollBack = [](GLFWwindow *w, double xoffset, double yoffset){
        //yic::systemHub.enqueueEvent(ev::glScrollInput{xoffset, yoffset});
        //yic::systemHub.publish(ev::glScrollInput{xoffset, yoffset});
    };

    inline auto setWindowPosCallback = [](GLFWwindow *w, int xpos, int ypos) {

    };

    inline auto setWindowSizeCallback = [](GLFWwindow *w, int width, int height) {

    };

    inline auto setWindowFocusCallback = [](GLFWwindow *w, int focused) {

    };

    inline auto setWindowCloseCallback = [](GLFWwindow* w){
        auto win = static_cast<Window*>(glfwGetWindowUserPointer(w));
        win->onClose();
        glfwSetKeyCallback(w, nullptr);
        glfwSetMouseButtonCallback(w, nullptr);
        glfwSetCursorPosCallback(w, nullptr);
    };

    inline auto setDropCallback = [](GLFWwindow *w, int count, const char** paths){
        if (count){
            vot::vector<vot::string> pts(count);
            for(auto i = 0; i < count; i++){
                pts[i] = paths[i];
            }
            yic::systemHub.publishAsync(ev::tResourcesPaths{ pts });
        }
    };

    Window::Window(const int &w, const int &h, vot::string name) : mWidth(w), mHeight(h), mName(std::move(name)), mWindow(createWindow()){
        yic::systemHub.sto(ev::pWindowContext{mWindow});
        yic::systemHub.setEvent(ev::oWindowSizeChange{vk::Extent2D{(uint32_t) w, (uint32_t) h}});
    }

    Window::~Window() {
        glfwDestroyWindow(mWindow);
        glfwTerminate();
    }

    auto Window::createWindow() -> GLFWwindow * {
            if (!glfwInit())
                return nullptr;
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RELEASE, GLFW_TRUE);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        return glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
    }

    auto Window::loop(const std::function<void()> &fn) const -> bool {
        glfwSetWindowUserPointer(mWindow, (void *) this);
        glfwSetKeyCallback(mWindow, setKeyCallback);
        glfwSetMouseButtonCallback(mWindow, setMouseButtonCallback);
        glfwSetCursorPosCallback(mWindow, setCursorPosCallback);

        glfwSetFramebufferSizeCallback(mWindow, framebufferSizeCallback);
        glfwSetWindowPosCallback(mWindow, setWindowPosCallback);
        glfwSetWindowSizeCallback(mWindow, setWindowSizeCallback);
        glfwSetScrollCallback(mWindow, setScrollBack);
        glfwSetDropCallback(mWindow, setDropCallback);

        glfwSetWindowCloseCallback(mWindow, setWindowCloseCallback);

        try {
            while(true){
                if (closeRequested.load(std::memory_order_relaxed) && closeRender.load(std::memory_order_relaxed)) return true;

                fn();

                const_cast<Window*>(this)->controller();

                glfwPollEvents();
                //glfwWaitEvents();
            }
        } catch (std::exception& e){
            std::cerr << e.what() << "\n";
        }

        return false;
    }


    auto Window::controller()  -> void {
        auto keyPress = [&](int key){
            return (glfwGetKey(mWindow, key) == GLFW_PRESS);
        };
        auto glfwMouseButtonPress = [&](int button){
            return (glfwGetMouseButton(mWindow, button) == GLFW_PRESS);
        };
        auto glfwMouseButtonRelease = [&](int button){
            return (glfwGetMouseButton(mWindow, button) == GLFW_RELEASE);
        };


        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            closeRequested.store(true, std::memory_order_relaxed);
            glfwSetWindowShouldClose(mWindow, GLFW_FALSE);
        }

        if (!isRightMouseDown) {
            if (keyPress(GLFW_KEY_S)) {
                GLOBAL::gizmoOperation = ImGuizmo::SCALE;
            }
            if (keyPress(GLFW_KEY_R)) {
                GLOBAL::gizmoOperation = ImGuizmo::ROTATE;
            }
            if (keyPress(GLFW_KEY_T)) {
                GLOBAL::gizmoOperation = ImGuizmo::TRANSLATE;
            }

        }

        if (glfwMouseButtonPress(GLFW_MOUSE_BUTTON_RIGHT)){
            isRightMouseDown = true;

            if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS){
                yic::systemHub.sto(ev::freeCameraController{.W = true});
            }
            if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS){
                yic::systemHub.sto(ev::freeCameraController{.A = true});
            }
            if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS){
                yic::systemHub.sto(ev::freeCameraController{.S = true});
            }
            if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS){
                yic::systemHub.sto(ev::freeCameraController{.D = true});
            }

            if (firstClick){
                glfwSetCursorPos(mWindow, xLast, yLast);
                firstClick = false;
            }
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xPos, double yPos){
                yic::systemHub.sto(ev::freeCameraController{.cursor = true, .xPos = xPos, .yPos = yPos});
            });
            glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xOffset, double yOffset){
                yic::systemHub.sto(ev::freeCameraController{.scroll = true, .xOffset = xOffset, .yOffset = yOffset});
            });

        }
        if (glfwMouseButtonRelease(GLFW_MOUSE_BUTTON_RIGHT)){
            glfwGetCursorPos(mWindow, &xLast, &yLast);
            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetCursorPosCallback(mWindow, setCursorPosCallback);
            glfwSetScrollCallback(mWindow, setScrollBack);
            firstClick = true;
            isRightMouseDown = false;
            yic::systemHub.sto(ev::freeCameraController{.firstM = true});
//            sc::globalCamera.firstMouse = true;
        }
    }

} // yic