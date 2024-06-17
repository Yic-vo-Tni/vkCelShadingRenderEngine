//
// Created by lenovo on 5/21/2024.
//

#include "vkWindow.h"

namespace yic {

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_NCHITTEST:
                return HTTRANSPARENT;
            case WM_SIZE:
                if (wParam == SIZE_MINIMIZED){
                    ShowWindow(hwnd, SW_HIDE);
                } else {
                    ShowWindow(hwnd, SW_SHOW);
                }
                return 0;
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
            case WM_ERASEBKGND:
                return 1;
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    vkWindow::vkWindow(const int &w, const int &h) : mWidth{w}, mHeight{h},
                                                     mWindow(createWindow()){

        EventBus::update(et::vkRenderContext{
                std::make_pair(mWidth, mHeight), vk::Extent2D{(uint32_t)mWidth, (uint32_t)mHeight}, mWindow.get()
        });

//        EventBus::subscribeAuto([&](const et::vkRenderContext &vkRenderContext) {
//            vkTrance("width: {0}, height: {1}", vkRenderContext.width_v(), vkRenderContext.height_v());
//        });
    }

    auto vkWindow::createOverlayWindow() -> void {
        auto hwndMain = glfwGetWin32Window(get()->mWindow.get());
        auto hInstance = GetModuleHandle(nullptr);
        const char CLASS_NAME[] = "OverlayWindowClass";
        registerWindowClass(CLASS_NAME, hInstance);

        get()->mHwndOverlayWindow = createOverlayWindow(CLASS_NAME, hInstance, get()->mWidth, get()->mHeight, hwndMain);
        glfwSetWindowUserPointer(get()->mWindow.get(), (void*)get()->mHwndOverlayWindow);
        SetLayeredWindowAttributes(get()->mHwndOverlayWindow, RGB(0, 0, 0), 0, LWA_ALPHA);
        ShowWindow(get()->mHwndOverlayWindow, SW_SHOW);

        EventBus::update(et::vkRenderContext{
            std::make_pair(get()->mWidth, get()->mHeight),
            vk::Extent2D{(uint32_t)get()->mWidth, (uint32_t)get()->mHeight},
            get()->mHwndOverlayWindow
        }, et::vkRenderContext::id::mainRender);
    }

    bool vkWindow::run() {
        try {
            while (!glfwWindowShouldClose(GetWindow())) {
                glfwPollEvents();

                TaskBus::executeTask<tt::EngineFlow>();

                MSG msg;
                while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)){
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }

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

    auto vkWindow::createOverlayWindow(const char* className, HINSTANCE hInstance, int width, int height, HWND parent) -> HWND  {
        int xpos, ypos;
        glfwGetWindowPos(get()->mWindow.get(), &xpos, &ypos);

        return CreateWindowEx(
                WS_EX_LAYERED | WS_EX_TRANSPARENT,
                className,
                "Overlay Window",
                WS_POPUP,
                xpos, ypos, width, height,
                parent,
                nullptr, hInstance, nullptr
        );
    }

    auto vkWindow::registerWindowClass(const char *className, HINSTANCE hInstance) -> void {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = className;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
        RegisterClass(&wc);
    }

} // yic
























