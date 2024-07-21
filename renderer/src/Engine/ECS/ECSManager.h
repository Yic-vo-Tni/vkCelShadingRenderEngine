//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_ECSMANAGER_H
#define VKCELSHADINGRENDERER_ECSMANAGER_H

#include "Camera/Camera.h"

namespace sc {

    class ECSManager {
    public:
        vkGet auto get = []{ return Singleton<ECSManager>::get(); };
        ECSManager();

        auto test() -> void;
        auto run() -> void;
    private:
        flecs::world ecs;
    };

} // sc

#endif //VKCELSHADINGRENDERER_ECSMANAGER_H
