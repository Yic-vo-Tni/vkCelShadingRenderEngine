//
// Created by lenovo on 10/31/2024.
//

#ifndef VKCELSHADINGRENDERER_RESOURCESYSTEM_H
#define VKCELSHADINGRENDERER_RESOURCESYSTEM_H

#include "Load/Loader.h"
#include "Load/Animator.h"

namespace rs {

    class ResourceSystem {
    public:
        //explicit ResourceSystem(flecs::world& world);
        explicit ResourceSystem(entt::registry& registry);
        ~ResourceSystem();
        MAKE_SINGLETON(ResourceSystem);

        auto frame() -> void;
    public:
    private:
        //flecs::world& ecs;
        entt::registry& ecs;
        double mSaveTime{};
        float mElapsed{}, mAnimTime{0.f};
        int lastFrame{0};
    public:
        std::unique_ptr<Loader> mLoader;
        std::unique_ptr<Animator> mAnimator;
    };


} // rs

namespace yic{
    inline rs::ResourceSystem* resourceSystem;
}

#endif //VKCELSHADINGRENDERER_RESOURCESYSTEM_H
