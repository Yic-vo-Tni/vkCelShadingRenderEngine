//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_VKWINDOW_H
#define VKCELSHADINGRENDERER_VKWINDOW_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Core/Callback/GlfwCallback.h"
#include "Engine/Core/Input/InputHandlers.h"
#include "SFML/System.hpp"

namespace yic {

    class vkWindow : public nonCopyable{
        struct Config{
            bool resizable = {true};
            bool transparentFramebuffer = true;
            bool visible = true;
            bool decorated = true;
            bool focused = true;
            bool maximized = false;
        };
    public:
        vkGet auto get = [](const int& w = 2000, const int& h = 1200){ return Singleton<vkWindow>::get(w, h);};

        vkWindow(const int& w, const int& h);
        ~vkWindow() = default;

        static bool run();

        [[nodiscard]] inline static auto GetWindow()  { return get()->mWindow.get();}

    private:
        auto createWindow() -> std::shared_ptr<GLFWwindow>;
    private:
        int mWidth{}, mHeight{};
        std::string mName{"Yicvot"};

        std::shared_ptr<GLFWwindow> mWindow;
    private:
        auto beginFrame() -> void;
        auto endFrame() -> void;
        sf::Clock mClock;
        sf::Time mStart, mFrameTime, mTimePerFrame;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKWINDOW_H
