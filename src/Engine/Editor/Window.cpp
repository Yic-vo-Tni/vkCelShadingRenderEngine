//
// Created by lenovo on 9/24/2024.
//

#include "Window.h"
#include "Core/DispatchSystem/SystemHub.h"

#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace yic {

    inline auto framebufferSizeCallback = [](GLFWwindow *w, int width, int height) {

    };

    inline auto setKeyCallback = [](GLFWwindow *w, int key, int scancode, int action, int mods) {
        systemHub.frame_write(ev::glKeyInput{key, scancode, action, mods});

        if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
            GLOBAL::visibleZMO = !GLOBAL::visibleZMO;
        }
    };

    inline auto setCharCallback = [](GLFWwindow *w, unsigned int code) {
        systemHub.frame_write(ev::glCharInput{code});
    };

    inline auto setMouseButtonCallback = [](GLFWwindow *w, int button, int action, int mods) {
        systemHub.frame_write(ev::glMouseInput{button, action, mods});
    };

    inline auto setCursorPosCallback = [](GLFWwindow*w, double xpos, double ypos){
        systemHub.frame_write(ev::glCursorPosInput{xpos, ypos});
    };

    inline auto setScrollBack = [](GLFWwindow *w, double xoffset, double yoffset) {
        systemHub.frame_write<ev::glScrollInput>({xoffset, yoffset});
    };

    inline auto setWindowPosCallback = [](GLFWwindow *w, int xpos, int ypos) {

    };

    inline auto setWindowSizeCallback = [](GLFWwindow *w, int width, int height) {

    };

    inline auto setWindowFocusCallback = [](GLFWwindow *w, int focused) {

    };

    inline auto setWindowCloseCallback = [](GLFWwindow* w){
        const auto win = static_cast<Window*>(glfwGetWindowUserPointer(w));
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
            yic::systemHub.pub_async(ev::tResourcesPaths{ pts });
        }
    };

    Window::Window(vot::string name) : mName(std::move(name)), mWindow(createWindow()){
        yic::systemHub.sto(ev::pWindowContext{mWindow});
        yic::systemHub.setEvent(ev::oWindowSizeChange{vk::Extent2D{static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)}});
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

        yic::systemHub.sto(ev::vFreeCameraController{}); // init

        compWindowSize();

        return glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
    }

    auto Window::loop(const std::function<void()> &fn) const -> bool {
        glfwSetWindowUserPointer(mWindow, (void *) this);
        glfwSetKeyCallback(mWindow, setKeyCallback);
        glfwSetMouseButtonCallback(mWindow, setMouseButtonCallback);
        glfwSetCharCallback(mWindow, setCharCallback);
        glfwSetCursorPosCallback(mWindow, setCursorPosCallback);

        glfwSetFramebufferSizeCallback(mWindow, framebufferSizeCallback);
        glfwSetWindowPosCallback(mWindow, setWindowPosCallback);
        glfwSetWindowSizeCallback(mWindow, setWindowSizeCallback);
        glfwSetScrollCallback(mWindow, setScrollBack);
        glfwSetDropCallback(mWindow, setDropCallback);

        glfwSetWindowCloseCallback(mWindow, setWindowCloseCallback);

        setWindowIcon(mWindow, tex_path "icon.jpg");

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
        auto keyPress = [&](const int key){ return (glfwGetKey(mWindow, key) == GLFW_PRESS); };
        auto glfwMouseButtonPress = [&](const int button){ return (glfwGetMouseButton(mWindow, button) == GLFW_PRESS); };
        auto glfwMouseButtonRelease = [&](const int button){ return (glfwGetMouseButton(mWindow, button) == GLFW_RELEASE); };

        static int lastDeleteState = GLFW_RELEASE;
        static int lastXState      = GLFW_RELEASE;

        int currentDelete = glfwGetKey(mWindow, GLFW_KEY_DELETE);
        int currentX      = glfwGetKey(mWindow, GLFW_KEY_X);

        if (currentDelete == GLFW_PRESS && lastDeleteState == GLFW_RELEASE) {
            yic::systemHub.pub_enqueue(ev::tDestroyEntity{});
            yic::logger->warn("Delete pressed once");
        }

        if (currentX == GLFW_PRESS && lastXState == GLFW_RELEASE) {
            yic::systemHub.pub_enqueue(ev::tDestroyEntity{});
            yic::logger->warn("X pressed once");
        }

        lastDeleteState = currentDelete;
        lastXState      = currentX;

        if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            closeRequested.store(true, std::memory_order_relaxed);
            glfwSetWindowShouldClose(mWindow, GLFW_FALSE);
        }

        if (!isRightMouseDown) {
            if (keyPress(GLFW_KEY_S)) { GLOBAL::gizmoOperation = ImGuizmo::SCALE; }
            if (keyPress(GLFW_KEY_R)) { GLOBAL::gizmoOperation = ImGuizmo::ROTATE; }
            if (keyPress(GLFW_KEY_T)) { GLOBAL::gizmoOperation = ImGuizmo::TRANSLATE; }
        }

        {
            if (auto f_Lock = yic::systemHub.vaL<ev::vFreeCameraController>(); glfwMouseButtonPress(GLFW_MOUSE_BUTTON_RIGHT)) {
                if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS) { f_Lock->W = true; }
                if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS) { f_Lock->A = true; }
                if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS) { f_Lock->S = true; }
                if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS) { f_Lock->D = true; }
            }
        }

        if (glfwMouseButtonPress(GLFW_MOUSE_BUTTON_RIGHT)) {
            auto controller = yic::systemHub.vaL<ev::vFreeCameraController>();
            isRightMouseDown = true;

            if (firstClick) {
                glfwSetCursorPos(mWindow, xLast, yLast);
                firstClick = false;
            }

            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            glfwSetCursorPosCallback(mWindow, [](GLFWwindow *window, double xPos, double yPos) {
                auto f_Lock = yic::systemHub.vaL<ev::vFreeCameraController>();
                f_Lock->cursor = true;
                f_Lock->xPos = xPos;
                f_Lock->yPos = yPos;
            });

            glfwSetScrollCallback(mWindow, [](GLFWwindow *window, double xOffset, double yOffset) {
                auto f_Lock = yic::systemHub.vaL<ev::vFreeCameraController>();
                f_Lock->scroll = true;
                f_Lock->xOffset = xOffset;
                f_Lock->yOffset = yOffset;
            });
        }

        if (glfwMouseButtonRelease(GLFW_MOUSE_BUTTON_RIGHT)) {
            auto controller = yic::systemHub.vaL<ev::vFreeCameraController>();

            glfwGetCursorPos(mWindow, &xLast, &yLast);

            glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

            glfwSetCursorPosCallback(mWindow, setCursorPosCallback);
            glfwSetScrollCallback(mWindow, setScrollBack);

            firstClick = true;
            isRightMouseDown = false;
            controller->firstM = true;
        }

    }

    auto Window::setWindowIcon(GLFWwindow* window, const char* filename) const -> void {
        int width, height, channels;
        unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 4);
        if (!pixels) {
            printf("Failed to load icon: %s\n", filename);
            return;
        }

        GLFWimage images[1];
        images[0].width  = width;
        images[0].height = height;
        images[0].pixels = pixels;

        glfwSetWindowIcon(window, 1, images);

        stbi_image_free(pixels);
    }

    auto Window::compWindowSize() -> void {
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        if (!primary) {

        }

        const GLFWvidmode* mode = glfwGetVideoMode(primary);

        int workX, workY, workW, workH;
        glfwGetMonitorWorkarea(primary, &workX, &workY, &workW, &workH);

        constexpr float aspect = 5.f / 3.f;

        mHeight = static_cast<int>(workH * 0.8f);
        mWidth = static_cast<int>(mHeight * aspect);

        if (mWidth > workW * 0.9f) {
            mWidth = static_cast<int>(workW * 0.9);
            mHeight = static_cast<int>(mWidth / aspect);
        }
    }
} // yic