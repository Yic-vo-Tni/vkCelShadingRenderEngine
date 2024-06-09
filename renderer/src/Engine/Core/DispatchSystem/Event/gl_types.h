//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_GL_TYPES_H
#define VKCELSHADINGRENDERER_GL_TYPES_H


namespace et{

    struct WindowContext {
        struct_y(WindowContext,
        (opt<std::pair<int, int>>, size),
        (opt<vk::Extent2D>, extent),
        (opt<GLFWwindow*>, window));
    };

    struct KeyInput {
        struct_y(KeyInput,
        (opt<int>, key),
        (opt<int>, action),
        (opt<int>, scancode),
        (opt<int>, mods));
    };

}

#endif //VKCELSHADINGRENDERER_GL_TYPES_H
