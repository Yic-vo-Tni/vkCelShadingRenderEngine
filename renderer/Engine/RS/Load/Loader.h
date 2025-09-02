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
#include <barrier>

namespace rs {

    class Loader{
    public:
        explicit Loader(entt::registry& registry);
        ~Loader();

    public:
      auto gVmdFiles() const { return mMmdLoader->vmdFiles; }

    private:
        auto asyncLoadA() -> void;

    private:
        auto onResourcePaths(const vot::string& pt) -> void;
        auto LoadModel(const vot::string& pt) -> void;
        auto onModelLoaded(const ev::tModelLoaded& ev) -> void;
        auto onModelLoadedS() -> void;

        auto check(const vot::string& pt, const vot::vector<vot::string>& suffixes) -> bool {
            return std::ranges::any_of(suffixes, [&](const vot::string& suffix){
                 return pt.ends_with(suffix);
             });
        }
    private:
        entt::registry& ecs;
        ev::pVkSetupContext ct{};

        std::barrier<> sync_point;
    public:
        std::unique_ptr<AssimpLoader> mAssimpLoader;
        std::unique_ptr<MmdLoader> mMmdLoader;
        std::unique_ptr<Audio> mAudio;
    };



} // rs

#endif //VKCELSHADINGRENDERER_LOADER_H
