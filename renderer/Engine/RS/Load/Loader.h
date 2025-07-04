//
// Created by lenovo on 10/31/2024.
//

#ifndef VKCELSHADINGRENDERER_LOADER_H
#define VKCELSHADINGRENDERER_LOADER_H

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Utils/Auxiliary.h"
#include "Animation.h"
#include "AssimpLoader.h"
#include "MmdLoader.h"
#include "Audio.h"

namespace rs {

    class Loader{
    public:
        explicit Loader(entt::registry& registry);
        //explicit Loader(flecs::world& world);
        ~Loader();

    private:
        auto asyncLoadA() -> void;
    private:
        //flecs::world& ecs;
        entt::registry& ecs;
        entt::entity e;
        ev::pVkSetupContext ct{};
        std::atomic<bool> _readyA{false}, _doneA{false};
    public:
        std::unique_ptr<AssimpLoader> mAssimpLoader;
        std::unique_ptr<MmdLoader> mMmdLoader;
        std::unique_ptr<Audio> mAudio;
    };



} // rs

#endif //VKCELSHADINGRENDERER_LOADER_H
