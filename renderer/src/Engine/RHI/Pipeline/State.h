//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_STATE_H
#define VKCELSHADINGRENDERER_STATE_H

#include "Engine/Utils/Log.h"

namespace yic{
    template<typename I>
    struct State{
        void init(){
            static_cast<I*>(this)->initSpecific();
        }

    };
}


#endif //VKCELSHADINGRENDERER_STATE_H
