//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_VKWINDOW_H
#define VKCELSHADINGRENDERER_VKWINDOW_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Core/Callback/GlfwCallback.h"
#include "Engine/Core/Input/InputHandlers.h"

namespace yic {

    class vkWindow : public nonCopyable{
    public:
        vkGet auto get = [](const int& w = 1600, const int& h = 1200){ return Singleton<vkWindow>::get(w, h);};

        vkWindow(const int& w, const int& h);
        ~vkWindow() = default;

        static bool run();

        [[nodiscard]] inline static auto GetWindow()  { return get()->mWindow.get();}

    private:
        auto createWindow() -> std::shared_ptr<GLFWwindow>;
        [[nodiscard]] static auto createOverlayWindow(const char* className, HINSTANCE hInstance, int width, int height, HWND parent) ->  HWND ;
        static auto registerWindowClass(const char* className, HINSTANCE hInstance) -> void;
    private:
        int mWidth{}, mHeight{};
        std::string mName{"Yicvot"};

        std::shared_ptr<GLFWwindow> mWindow;
        HWND mHwndOverlayWindow{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKWINDOW_H
