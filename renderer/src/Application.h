//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_APPLICATION_H
#define VKCELSHADINGRENDERER_APPLICATION_H

#include "Editor/Editor.h"

namespace yic {

    class Application : public nonCopyable{
    public:
        Application();
        void run();

    private:
        std::unique_ptr<Editor> mEditor;
    };

} // yic

#endif //VKCELSHADINGRENDERER_APPLICATION_H
