//
// Created by lenovo on 7/19/2024.
//

#ifndef VKCELSHADINGRENDERER_ECSMANAGER_H
#define VKCELSHADINGRENDERER_ECSMANAGER_H

#include "Camera/Camera.h"
#include "Model/ModelManager.h"




namespace sc {

    class ECSManager {
    public:
        ECSManager();
        ~ECSManager();

        auto clear() -> void{
            mModelManager.reset();
            ecs.reset();
        }

//        auto Render(vk::CommandBuffer& cmd) -> void;
//        auto renderStorage(vk::CommandBuffer& cmd) -> void;
        auto prepare() -> void;
    private:
        flecs::world ecs;

        std::unique_ptr<ModelManager> mModelManager;


    };

} // sc

#endif //VKCELSHADINGRENDERER_ECSMANAGER_H
