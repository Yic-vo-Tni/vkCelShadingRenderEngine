//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_GLFWCALLBACK_H
#define VKCELSHADINGRENDERER_GLFWCALLBACK_H

#include "Engine/Core/DispatchSystem/Schedulers.h"


namespace glfw_callback {

    inline auto framebufferSizeCallback = [](GLFWwindow *window, int width, int height) {
        yic::EventBus::publish(et::WindowContext{
                .size = std::make_pair(width, height),
                .extent = vk::Extent2D{(uint32_t) width, (uint32_t) height},
        });

    };

    inline auto setKeyCallback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        yic::EventBus::publish(et::KeyInput{key, action, scancode, mods});

        auto& io = ImGui::GetIO();
        switch (action) {
            case GLFW_PRESS:
                io.KeysDown[key] = true;
                break;
            case GLFW_RELEASE:
                io.KeysDown[key] = false;
                break;
            default:
                break;
        }
        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    };

    inline auto setMouseButtonCallback = [](GLFWwindow *w, int button, int action, int mods){
        ImGui_ImplGlfw_MouseButtonCallback(w, button, action, mods);
    };

    inline auto setWindowPosCallback = [](GLFWwindow *w, int xpos, int ypos){
        HWND hwndOverlay = (HWND)glfwGetWindowUserPointer(w);
        if (hwndOverlay) {
            SetWindowPos(hwndOverlay, HWND_BOTTOM, xpos, ypos, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
        }
    };

    inline auto setWindowSizeCallback = [](GLFWwindow* w, int width , int height){
        HWND hwndOverlay = (HWND)glfwGetWindowUserPointer(w);
        if (hwndOverlay) {
            SetWindowPos(hwndOverlay, HWND_BOTTOM, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
        }
    };

}

inline auto callback = [](GLFWwindow* w){
    glfwSetFramebufferSizeCallback(w, glfw_callback::framebufferSizeCallback);
    glfwSetKeyCallback(w, glfw_callback::setKeyCallback);
    glfwSetMouseButtonCallback(w, glfw_callback::setMouseButtonCallback);
    glfwSetWindowPosCallback(w, glfw_callback::setWindowPosCallback);
    glfwSetWindowSizeCallback(w, glfw_callback::setWindowSizeCallback);
};

#endif //VKCELSHADINGRENDERER_GLFWCALLBACK_H
