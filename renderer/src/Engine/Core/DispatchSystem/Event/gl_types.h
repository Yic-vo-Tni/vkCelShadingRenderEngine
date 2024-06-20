//
// Created by lenovo on 6/3/2024.
//

#ifndef VKCELSHADINGRENDERER_GL_TYPES_H
#define VKCELSHADINGRENDERER_GL_TYPES_H

namespace wrapper{

}

namespace et{

    struct glKeyInput {
        struct_y(glKeyInput,
                 (opt<int>, key),
                 (opt<int>, action),
                 (opt<int>, scancode),
                 (opt<int>, mods));
    } ;

    struct glMouseInput {
        struct_y(glMouseInput,
                 (opt<int>, button),
                 (opt<int>, action),
                 (opt<int>, modes));
    } ;

    struct glCursorPosInput {
        struct_y(glCursorPosInput,
                 (opt<double>, xpos),
                 (opt<double>, ypos));
    } ;

    struct glScrollInput {
        struct_y(glScrollInput,
                 (opt<double>, xoffset),
                 (opt<double>, yoffset));
    } ;

}

#endif //VKCELSHADINGRENDERER_GL_TYPES_H
