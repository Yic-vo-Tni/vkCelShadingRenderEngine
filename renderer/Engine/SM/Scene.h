//
// Created by lenovo on 11/21/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

#include "Core/Management/TripleBufferIndexManager.h"
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

        auto syncBLAS(const vot::VertexDataComponent &vc, const vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
                       bool update = false) -> void;
        // auto syncBLAS(const std::variant<vot::VertexDataComponent<vot::eAssimp>, vot::VertexDataComponent<vot::eMMD>> &vdc, const vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
        //                bool update = false) -> void;

        // template<vot::VertexType type>
        // auto syncBLAST(const vot::VertexDataComponent<type> &vdc, vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
        //                bool update = false) -> void {
        //     auto maxVert = static_cast<uint32_t>(vdc.vertices_pmr[yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur()].size());
        //     auto numTri = static_cast<uint32_t>(vdc.indices_pmr.size()) / 3;
        // };

        auto syncTLAS() -> void;
        auto acquireActiveScene() { return mActiveScene; }
    private:
        // auto syncBLAS(const std::pair<uint32_t, uint32_t>& num, vot::RenderComponent &rc, vot::RayTracingComponent &rtc,
        //        bool update = false) -> void;

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
