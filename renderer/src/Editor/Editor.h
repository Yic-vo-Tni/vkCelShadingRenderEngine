//
// Created by lenovo on 5/23/2024.
//

#ifndef VKCELSHADINGRENDERER_EDITOR_H
#define VKCELSHADINGRENDERER_EDITOR_H

#include "Engine/Engine.h"

namespace yic {

    class Editor {
    public:
        Editor();

        void run() ;

    private:
        std::unique_ptr<Engine> mEngine;
    };

} // yic

#endif //VKCELSHADINGRENDERER_EDITOR_H
