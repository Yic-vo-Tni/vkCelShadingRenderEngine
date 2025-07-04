//
// Created by lenovo on 11/21/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

#include "RHI/Descriptor.h"

namespace sm {

    struct Scene {
        explicit Scene(vot::string id) : id(std::move(id)){}

        vot::Accel_sptr tlas;
        vot::Descriptor_sptr descriptor;
        vot::Buffer_sptr bufferAddrBuffer;

        vot::Buffer_sptr instBuffer;
        vot::Buffer_sptr scratchBuffer;
        bool update = false;
    private:
        vot::string id;
    };

    class SceneSystem{
    public:
        MAKE_SINGLETON(SceneSystem);
        //explicit SceneSystem(flecs::world& ecs);
        explicit SceneSystem(entt::registry& registry);
        ~SceneSystem() = default;

        auto loadScene(const vot::string& id = {}) -> void;
        auto unloadScene(const vot::string& id) -> void;

        auto reloadTlas() -> void;

        auto frame() -> void;

        auto syncBLAS(const vot::VertexDataComponent &vc, vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
                       bool update = false) -> void;
        auto syncTLAS() -> void;
        auto acquireActiveScene() { return mActiveScene; }
    private:
        auto updateDS() -> void;
    private:
//        flecs::world& ecs;
        entt::registry& ecs;
        ev::pVkSetupContext ct{};
        ev::pVkRenderContext rt{};

        Scene* mActiveScene{};
        vot::unordered_map<vot::string, Scene> mScenes;
    };

} // sm

namespace yic{
    inline sm::SceneSystem* sceneSystem;
}

#endif //VKCELSHADINGRENDERER_SCENE_H
