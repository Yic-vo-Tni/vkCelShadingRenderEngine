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

        yic::systemHub.sub_queued(2, [&](const ev::tModelLoaded& ev){ onModelLoaded(ev); });
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
            vertexDataComponent.type = vot::eMMD;
            mMmdLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent);
        } else if (check(pt, {".obj", ".fbx"})) {
            vertexDataComponent.type = vot::eAssimp;
            mAssimpLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
        } else if (check(pt, {".gltf", ".glb"})) {
            vertexDataComponent.type = vot::eAssimp;
            mAssimpLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
        }
        yic::sceneSystem->syncBLAS(vertexDataComponent, renderComponent, rayTracingComponent);
        GLOBAL::pickON = basicInfoComponent.name;

        yic::systemHub.pub_enqueue(ev::tModelLoaded{
            basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent
        });

    }


    auto Loader::onModelLoaded(const ev::tModelLoaded& ev) -> void {
        const auto entity = ecs.create();
        auto& [basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent] = ev;

        ecs.emplace<vot::BasicInfoComponent>(entity, basicInfoComponent);
        ecs.emplace<vot::VertexDataComponent>(entity, vertexDataComponent);
        ecs.emplace<vot::RenderComponent>(entity, renderComponent);
        ecs.emplace<vot::AnimationComponent>(entity, animationComponent);
        ecs.emplace<vot::RayTracingComponent>(entity, rayTracingComponent);

        // if (vertexDataComponent.isMMD)
        //     ecs.emplace<vot::mark::eMMD>(entity);
        if (vertexDataComponent.type == vot::eMMD)
            ecs.emplace<vot::mark::eMMD>(entity);

        yic::sceneSystem->reloadTlas();

        ecs.emplace<vot::mark::eVisible>(entity);
    }

}



















