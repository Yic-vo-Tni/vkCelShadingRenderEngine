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
    //Loader::Loader(flecs::world& world) : ecs(world) {
        ct = yic::systemHub.val<ev::pVkSetupContext>();
        mAssimpLoader = std::make_unique<AssimpLoader>();
        mMmdLoader = std::make_unique<MmdLoader>();
        mAudio = std::make_unique<Audio>();
        asyncLoadA();
    }

    Loader::~Loader() = default;

    auto Loader::asyncLoadA() -> void {
        yic::systemHub.subscribe([&](const ev::tResourcesPaths &pts) {
            yic::logger->warn("Load path: {0}", pts.paths[0]);

            {
                _readyA.store(false, std::memory_order_relaxed);
                _doneA.store(false, std::memory_order_relaxed);
            }

            for (const auto &rawPt: pts.paths) {
                vot::string pt{rawPt};
                auto check = [&](const vot::vector<vot::string>& suffixes){
                    return std::ranges::any_of(suffixes, [&](const vot::string& suffix){
                        return pt.ends_with(suffix);
                    });
                };

                vot::BasicInfoComponent basicInfoComponent{};
                vot::VertexDataComponent vertexDataComponent{};
                vot::RenderComponent renderComponent{};
                vot::AnimationComponent animationComponent{};
                vot::RayTracingComponent rayTracingComponent{};

                if (check({".mp3"})){
                    mAudio->Load(pt);
                } else if (check({".vmd"})) {
                    mMmdLoader->vmd(pt);
                } else {
                    if (check({".pmx"})){
                        mMmdLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent);
                    } else if (check({".obj", ".fbx", ".gltf", ".glb"})) {
                        mAssimpLoader->Load(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
                    }
                    yic::sceneSystem->syncBLAS(vertexDataComponent, renderComponent, rayTracingComponent);
                    GLOBAL::pickON = basicInfoComponent.name;

                    yic::systemHub.publishPolling(ev::tModelLoaded{});

                    {
                        _readyA.wait(false);
                    }

                    e = ecs.create();
                    ecs.emplace<vot::BasicInfoComponent>(e, basicInfoComponent);
                    ecs.emplace<vot::VertexDataComponent>(e, vertexDataComponent);
                    ecs.emplace<vot::RenderComponent>(e, renderComponent);
                    ecs.emplace<vot::AnimationComponent>(e, animationComponent);
                    ecs.emplace<vot::RayTracingComponent>(e, rayTracingComponent);



//                    auto& e = ecs.entity(basicInfoComponent.name.data())
//                            .set(basicInfoComponent)
//                            .set(vertexDataComponent)
//                            .set(renderComponent)
//                            .set(animationComponent)
//                            .set(rayTracingComponent);
                    if (vertexDataComponent.isMMD)
                        //e.add<vot::MMDTag>();
                        ecs.emplace<vot::MMDTag>(e);

                    {
                        _doneA.store(true, std::memory_order_release);
                        _doneA.notify_one();
                    }

                }
            }
        });

        yic::systemHub.subscribePolling([&](const ev::tModelLoaded&){
            {
                _readyA.store(true, std::memory_order_release);
                _readyA.notify_one();
            }

            {
                _doneA.wait(false);
            }

            //auto e = ecs.entity(GLOBAL::pickON.c_str());

            yic::sceneSystem->reloadTlas();

           // e.add<vot::RenderVisibleTag>();
            ecs.emplace<vot::RenderVisibleTag>(e);
        });
    }

}



















