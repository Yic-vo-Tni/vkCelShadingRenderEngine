//
// Created by lenovo on 10/31/2024.
//

#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/Allocator.h"
#include "RHI/Command.h"
#include "SM/Scene.h"

#include "Loader.h"

namespace rs {

    constexpr size_t _1GB = 1024 * 1024 * 1024;

    Loader::Loader(entt::registry& registry) : ecs(registry), sync_point(2) {
        ct = yic::systemHub.val<ev::pVkSetupContext>();

        mAssimpLoader = std::make_unique<AssimpLoader>();
        mMmdLoader = std::make_unique<MmdLoader>();
        mAudio = std::make_unique<Audio>();

        asyncLoadA();
    }

    Loader::~Loader() = default;

    auto Loader::asyncLoadA() -> void {
        yic::systemHub.subscribe([&](const ev::tResourcesPaths &pts) {
            for (const auto &pt: pts.paths)  onResourcePaths(pt);
        });

        yic::systemHub.subscribePolling([&](const ev::tModelLoaded& ev){ onModelLoaded(ev); });
        yic::systemHub.subscribePolling([&](const ev::tModelLoadedSlow&){ onModelLoadedS(); });
    }

    auto Loader::onResourcePaths(const vot::string &pt) -> void {
        yic::logger->warn("Load path: {0}", pt);

        if (check(pt, {".mp3"})){
            mAudio->Load(pt);
        } else if (check(pt, {".vmd"})) {
            mMmdLoader->LoadVmd(pt);
        } else {
            LoadModel(pt);
        }
    }

    auto Loader::LoadModel(const vot::string& pt) -> void {
        vot::BasicInfoComponent basicInfoComponent{};
        vot::VertexDataComponent vertexDataComponent{};
        vot::RenderComponent renderComponent{};
        vot::AnimationComponent animationComponent{};
        vot::RayTracingComponent rayTracingComponent{};

        if (check(pt, {".pmx"})) {
            mMmdLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent);
        } else if (check(pt, {".obj", ".fbx", ".gltf", ".glb"})) {
            mAssimpLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
        }
        yic::sceneSystem->syncBLAS(vertexDataComponent, renderComponent, rayTracingComponent);
        GLOBAL::pickON = basicInfoComponent.name;

        yic::systemHub.publishPolling(ev::tModelLoaded{
            basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent
        });
        yic::systemHub.publishPolling(ev::tModelLoadedSlow{});

    }


    auto Loader::onModelLoaded(const ev::tModelLoaded& ev) -> void {
        const auto entity = ecs.create();
        auto& [basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent] = ev;

        ecs.emplace<vot::BasicInfoComponent>(entity, basicInfoComponent);
        ecs.emplace<vot::VertexDataComponent>(entity, vertexDataComponent);
        ecs.emplace<vot::RenderComponent>(entity, renderComponent);
        ecs.emplace<vot::AnimationComponent>(entity, animationComponent);
        ecs.emplace<vot::RayTracingComponent>(entity, rayTracingComponent);

        if (vertexDataComponent.isMMD)
            ecs.emplace<vot::mark::eMMD>(entity);

        yic::sceneSystem->reloadTlas();

        ecs.emplace<vot::mark::eVisible>(entity);

        sync_point.arrive_and_wait();
    }

    auto Loader::onModelLoadedS() -> void {
        sync_point.arrive_and_wait();
    }
}



















