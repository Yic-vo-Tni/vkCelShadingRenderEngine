//
// Created by lenovo on 7/19/2024.
//

#include "ECSManager.h"

namespace sc {
    ECSManager::ECSManager() {
        ecs.set_threads(4);

        ecs.component<Camera>();
//        ecs.component<Model::Generic>();
        ecs.component<Model>();
        ecs.component<Selected>();

        yic::EventBus::update(et::eEcs_ptr{&ecs});

        mModelManager = std::make_unique<ModelManager>(&ecs);


    }

    ECSManager::~ECSManager() {
        ecs.reset();
    }

    auto ECSManager::Render(vk::CommandBuffer &cmd) -> void {
        mModelManager->Render(cmd);
    }

    auto ECSManager::renderStorage(vk::CommandBuffer &cmd) -> void {
        mModelManager->renderRt(cmd);
    }


} // sc