//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_GLFWCALLBACK_H
#define VKCELSHADINGRENDERER_GLFWCALLBACK_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace glfw_callback {

    inline auto framebufferSizeCallback = [](GLFWwindow *w, int width, int height) {
        yic::EventBus::publish(et::vkRenderContext{
                .size = ImVec2((float)width, (float)height),
                .extent = vk::Extent2D{(uint32_t) width, (uint32_t) height},
        }, et::vkRenderContext::id::mainRender);
    };

    inline auto setKeyCallback = [](GLFWwindow *w, int key, int scancode, int action, int mods) {
        yic::EventBus::publish(et::glKeyInput{key, action, scancode, mods});
    };

    inline auto setMouseButtonCallback = [](GLFWwindow *w, int button, int action, int mods) {
        yic::EventBus::publish(et::glMouseInput{button, action, mods});
    };

    inline auto setCursorPosCallback = [](GLFWwindow*w, double xpos, double ypos){
        yic::EventBus::publish(et::glCursorPosInput{xpos, ypos});
    };

    inline auto setScrollBack = [](GLFWwindow *w, double xoffset, double yoffset){
        yic::EventBus::publish(et::glScrollInput{xoffset, yoffset});
    };

    inline auto setWindowPosCallback = [](GLFWwindow *w, int xpos, int ypos) {

    };

    inline auto setWindowSizeCallback = [](GLFWwindow *w, int width, int height) {

    };

    inline auto setWindowFocusCallback = [](GLFWwindow *w, int focused) {

    };

    inline auto dropCallback = [](GLFWwindow *w, int count, const char** paths){
        bool model{false};
        bool img{false};
        std::vector<std::string> modelPaths;
        std::vector<std::string> imgPaths;
        for(int i = 0; i < count; i++){
            std::string path(paths[i]);

            if (path.ends_with(".obj") || path.ends_with(".fbx") || path.ends_with(".pmx") || path.ends_with(".gltf")){
                model = true;
                modelPaths.emplace_back(path);
            }
        }

        if (model){
            yic::EventBus::publish(et::modelPath{modelPaths});
        }
    };

}

inline auto callback = [](GLFWwindow* w){
    glfwSetFramebufferSizeCallback(w, glfw_callback::framebufferSizeCallback);
    glfwSetKeyCallback(w, glfw_callback::setKeyCallback);
    glfwSetMouseButtonCallback(w, glfw_callback::setMouseButtonCallback);
    glfwSetWindowPosCallback(w, glfw_callback::setWindowPosCallback);
    glfwSetWindowSizeCallback(w, glfw_callback::setWindowSizeCallback);
    glfwSetCursorPosCallback(w, glfw_callback::setCursorPosCallback);
    glfwSetScrollCallback(w, glfw_callback::setScrollBack);
    glfwSetDropCallback(w, glfw_callback::dropCallback);
};

inline auto cameraCallback = [](GLFWwindow* w){
    glfwSetCursorPosCallback(w, glfw_callback::setCursorPosCallback);
    glfwSetScrollCallback(w, glfw_callback::setScrollBack);
};

#endif //VKCELSHADINGRENDERER_GLFWCALLBACK_H
