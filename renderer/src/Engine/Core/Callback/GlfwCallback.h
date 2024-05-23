//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_GLFWCALLBACK_H
#define VKCELSHADINGRENDERER_GLFWCALLBACK_H

#include "Engine/Core/Event/Event.h"


namespace glfw_callback {

    inline auto framebufferSizeCallback = [](GLFWwindow *window, int width, int height) {
        yic::EventBus::publish(EventTypes::WindowSize{width, height});
    };

    inline auto setKeyCallback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        yic::EventBus::publish(EventTypes::KeyInput{key, action, scancode, mods});
    };

}

inline auto callback = [](GLFWwindow* w){
    glfwSetFramebufferSizeCallback(w, glfw_callback::framebufferSizeCallback);
    glfwSetKeyCallback(w, glfw_callback::setKeyCallback);
};

#endif //VKCELSHADINGRENDERER_GLFWCALLBACK_H
