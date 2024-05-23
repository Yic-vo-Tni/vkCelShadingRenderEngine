//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTTYPES_H
#define VKCELSHADINGRENDERER_EVENTTYPES_H

namespace EventTypes {

    struct WindowSize {
        int w;
        int h;
    };

    struct KeyInput{
        int key;
        int action;
        int scancode;
        int mods;
    };


}

#endif //VKCELSHADINGRENDERER_EVENTTYPES_H
