//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_GL_TYPES_H
#define VKCELSHADINGRENDERER_GL_TYPES_H


namespace et{

//    struct WindowContext {
//        struct_y(WindowContext,
//                 (opt<std::pair<int, int>>, size),
//                 (opt<vk::Extent2D>, extent),
//                 (opt<std::variant<GLFWwindow*, HWND>>, window));
//
//        struct id{
//            static constexpr const char* mainRender{"mainRender"};
//        };
//
//        template<typename T>
//        [[nodiscard]] const auto& window_v(){
//            if (!window.has_value())
//                throw std::runtime_error("window is not initialized.");
//
//            return std::get<T>(window.value());
//        }
//
//        [[nodiscard]] const auto& window_v(){
//            if (!window.has_value())
//                throw std::runtime_error("window is not initialized.");
//
//            return window.value();
//        }
//    };

    struct KeyInput {
        struct_y(KeyInput,
                 (opt<int>, key),
                 (opt<int>, action),
                 (opt<int>, scancode),
                 (opt<int>, mods));
    };

    struct MouseInput{
        struct_y(MouseInput,
                 (opt<int>, button),
                 (opt<int>, action),
                 (opt<int>, modes));
    };

}

#endif //VKCELSHADINGRENDERER_GL_TYPES_H
