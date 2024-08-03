//
// Created by lenovo on 7/22/2024.
//

#ifndef VKCELSHADINGRENDERER_MODELMANAGER_H
#define VKCELSHADINGRENDERER_MODELMANAGER_H

#include "Engine/ECS/Model/LoadModel.h"

namespace sc {

    class ModelManager {
    public:
        ModelManager(const flecs::world& esc) ;

    private:
        const flecs::world& esc;
    };

} // sc

#endif //VKCELSHADINGRENDERER_MODELMANAGER_H
