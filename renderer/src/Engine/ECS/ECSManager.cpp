//
// Created by lenovo on 7/19/2024.
//

#include "ECSManager.h"

namespace sc {
    ECSManager::ECSManager() {
        ecs.set_threads(4);

        ecs.component<Camera>();
        ecs.component<Model>();
        ecs.component<Selected>();

        yic::EventBus::update(et::eEcs_ptr{&ecs});

        globalCamera.initBuf();
        mModelManager = std::make_unique<ModelManager>(&ecs);


    }

    ECSManager::~ECSManager() {
        ecs.reset();
        globalCamera.clear();
    }

    auto ECSManager::prepare() -> void {
        mModelManager->prepare();
    }



} // sc