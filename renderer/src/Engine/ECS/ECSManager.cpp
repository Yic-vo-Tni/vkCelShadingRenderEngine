//
// Created by lenovo on 7/19/2024.
//

#include "ECSManager.h"

namespace sc {
    ECSManager::ECSManager() {
        ecs.set_threads(4);
    }

    auto ECSManager::test() -> void {
        ecs.component<Camera>();

        auto camera = ecs.entity("camera")
                .set<Camera>({});

        ecs.system<Camera>()
                .each([](flecs::entity e, Camera& camera){
                    camera.computeViewProjMatrix();
                });


    }

    auto ECSManager::run() -> void {
        ecs.progress();
    }


} // sc