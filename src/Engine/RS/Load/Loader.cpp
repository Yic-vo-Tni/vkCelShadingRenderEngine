//
// Created by lenovo on 10/31/2024.
//

#include "Core/DispatchSystem/SystemHub.h"
#include "SM/Scene.h"

#include "Loader.h"

namespace rs {

    constexpr size_t _1GB = 1024 * 1024 * 1024;

    Loader::Loader(entt::registry& registry) : ecs(registry) {
        ct = yic::systemHub.va<ev::pVkSetupContext>();

        mAssimpLoader = std::make_unique<AssimpLoader>();
        mMmdLoader = std::make_unique<MmdLoader>();
        mAudio = std::make_unique<Audio>();

        asyncLoadA();
    }

    Loader::~Loader() = default;

    auto Loader::asyncLoadA() -> void {
        yic::systemHub.sub([&](const ev::tResourcesPaths &pts) {
            for (const auto &pt: pts.paths)  onResourcePaths(pt);
        });

        yic::systemHub.sub_queued(2, [&](const ev::tModelLoaded &ev) { onModelLoaded(ev); });
        yic::systemHub.sub_queued(2, [&](const ev::tDestroyEntity &) { onEntityDestroyed(); });
    }

    auto Loader::onResourcePaths(const vot::string &pt) -> void {
        yic::logger->warn("Load path: {0}", pt);

        vot::dsl::Match{getExt(pt)}
            .case_(".mp3", [&]{ mAudio->Load(pt); })
            .case_(".vmd", [&]{ mMmdLoader->LoadVmd(pt); })
            .default_([&]{ LoadModel(pt); });
    }

    auto Loader::LoadModel(const vot::string& pt) -> void {
        vot::BasicInfoComponent basicInfo{};
        vot::VertexDataComponent vd{};
        vot::RenderComponent rc{};
        vot::AnimationComponent ac{};
        vot::RayTracingComponent rtc{};

        vot::dsl::Match{getExt(pt)}
            .case_(".pmx", [&]{ vd.type = vot::eMMD; mMmdLoader->Load(pt, basicInfo, vd, rc); })
            .case_({".obj", ".fbx"}, [&]{ vd.type = vot::eAssimp; mAssimpLoader->Load(pt, basicInfo, vd, rc, ac); })
            .case_({".gltf", ".glb"}, [&]{ vd.type = vot::eAssimp; mAssimpLoader->Load(pt, basicInfo, vd, rc, ac); });

        yic::sceneSystem->syncBLAS(vd, rc, rtc);
        GLOBAL::pickON = basicInfo.name;

        yic::systemHub.pub_enqueue(ev::tModelLoaded{basicInfo, vd, rc, ac, rtc});
    }


    auto Loader::onModelLoaded(const ev::tModelLoaded& ev) -> void {
        auto& [basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent] = ev;

        auto e = vot::EntityView<vot::bit::eEntityViewDefault>::create()
            .emplace(basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent)
            .mark_if<vot::mark::eAssimp, vot::mark::eMMD>(vertexDataComponent.type == vot::eAssimp);

        yic::sceneSystem->reloadTlas();

        e.mark<vot::mark::eVisible>();
    }

    auto Loader::onEntityDestroyed() const -> void {
        vot::vector<entt::entity> toD;

        ecs.view<vot::RenderComponent, const vot::BasicInfoComponent>().each(
            [&](const entt::entity e, vot::RenderComponent &, const vot::BasicInfoComponent &bic) {
                if (GLOBAL::pickON == bic.name && GLOBAL::visibleZMO) {
                    toD.emplace_back(e);
                }
            });

        for (const auto &e: toD) {
            yic::logger->info("Entity: {0}, valid ={1}", entt::to_integral(e), ecs.valid(e));
            ecs.destroy(e);
        }

        yic::sceneSystem->reloadTlas();
    }
}



















