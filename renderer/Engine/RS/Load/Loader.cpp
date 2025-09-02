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
        {
            _readyA.store(false, std::memory_order_relaxed);
            _doneA.store(false, std::memory_order_relaxed);
            _readyB.store(false, std::memory_order_relaxed);
            _doneB.store(false, std::memory_order_relaxed);
        }

        const auto end = std::chrono::high_resolution_clock::now();
        static auto last = end;
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - last).count();
        last = end;

        if (elapsed < 400'000) {
            std::this_thread::sleep_for(std::chrono::microseconds(400'000 - elapsed));
        }

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

        auto e = ecs.create();
        yic::systemHub.publishPolling(ev::tModelLoaded{e});
        yic::systemHub.publishPolling(ev::tModelLoadedSlow{});

        {
            _readyA.wait(false);
            _readyB.wait(false);
        }

        ecs.emplace<vot::BasicInfoComponent>(e, basicInfoComponent);
        ecs.emplace<vot::VertexDataComponent>(e, vertexDataComponent);
        ecs.emplace<vot::RenderComponent>(e, renderComponent);
        ecs.emplace<vot::AnimationComponent>(e, animationComponent);
        ecs.emplace<vot::RayTracingComponent>(e, rayTracingComponent);

        if (vertexDataComponent.isMMD)
            ecs.emplace<vot::mark::eMMD>(e);

        {
            _doneA.store(true, std::memory_order_release);
            _doneA.notify_one();

            _doneB.store(true, std::memory_order_release);
            _doneB.notify_one();
        }
    }


    auto Loader::onModelLoaded(const ev::tModelLoaded& ev) -> void {
        {
            _readyA.store(true, std::memory_order_release);
            _readyA.notify_one();
        }

        {
            _doneA.wait(false);
        }

        yic::sceneSystem->reloadTlas();

        ecs.emplace<vot::mark::eVisible>(ev.entity);
    }

    auto Loader::onModelLoadedS() -> void {
        {
            _readyB.store(true, std::memory_order_release);
            _readyB.notify_one();
        }

        {
            _doneB.wait(false);
        }
    }
}



















