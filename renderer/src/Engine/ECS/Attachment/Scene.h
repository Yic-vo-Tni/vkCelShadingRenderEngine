//
// Created by lenovo on 8/30/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

#include <utility>

#include "Engine/RHI/vkAsset.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/RHI/Descriptor.h"

namespace sc {

    struct Scene {
        explicit Scene(std::string id) : id(std::move(id)) {};

        std::string id;
        std::shared_ptr<yic::vkAccel> tlas;
        std::shared_ptr<yic::Descriptor> rtDesSet;
        std::vector<const Model*> models;
        std::vector<std::shared_ptr<yic::vkAccel>> blass;
    };

    class SceneManager{
    public:
        SceneManager();
        vkGet auto inst = []{ return Singleton<SceneManager>::get(); };
        auto switchScene(const std::string& id) -> void;
        auto loadScene(const std::string& id = {}) -> void;
        auto unloadScene(const std::string& id) -> void;
        auto addModel(const Model* model) -> void;
        auto updateScene() -> void;
        auto removeModel(const std::string& id) -> void;

    public:
        auto getActiveScene() { return mActiveScene; }

    private:
        auto cBlas(const Model* model) -> std::shared_ptr<yic::vkAccel>;
        auto cTlas(const std::vector<std::shared_ptr<yic::vkAccel>>& blass, std::shared_ptr<yic::vkAccel>& tlas) -> void;

    private:
        vk::Device mDevice;
        vk::DispatchLoaderDynamic mDyDispatcher;

        uint8_t mSceneCount{0};
        Scene* mActiveScene{};
        std::unordered_map<std::string, Scene> mScenes;
    };

} // sc

namespace Manager{
    inline auto Scene = sc::SceneManager::inst();
}

#endif //VKCELSHADINGRENDERER_SCENE_H
