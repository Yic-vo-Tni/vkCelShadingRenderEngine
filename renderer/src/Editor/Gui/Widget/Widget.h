#include <utility>

//
// Created by lenovo on 7/2/2024.
//

#ifndef VKCELSHADINGRENDERER_WIDGET_H
#define VKCELSHADINGRENDERER_WIDGET_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

#include "Editor/ShaderEditor/ShaderEditor.h"

namespace ui {

    using namespace yic;

    class Widget{
    public:
        explicit Widget(std::string  name) : mWindowName(std::move(name)){ };

        virtual auto record()->void = 0;

        auto getName() { return mWindowName; }
    private:
        std::string mWindowName;
    };



} // ui



#endif //VKCELSHADINGRENDERER_WIDGET_H
