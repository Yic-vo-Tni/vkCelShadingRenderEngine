//
// Created by lenovo on 5/20/2025.
//

#include "AsyncLoadSystem.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RenderLibrary.h"
#include "RS/ResourceSystem.h"
#include "SM/Scene.h"
#include "RHI/DescriptorSystem.h"

namespace sc {

//    AsyncLoadSystem::AsyncLoadSystem(flecs::world &world) : ecs(world) {
//        asyncLoadA();
//    }
//
//    auto AsyncLoadSystem::asyncLoadA() -> void {
//        yic::systemHub.subscribe([&](const ev::tResourcesPaths &pts) {
//            yic::logger->warn("Load path: {0}", pts.paths[0]);
//
//            {
//                _readyA.store(false, std::memory_order_relaxed);
//                _doneA.store(false, std::memory_order_relaxed);
//            }
//
//            for (const auto &rawPt: pts.paths) {
//                vot::string pt{rawPt};
//                auto check = [&](const vot::vector<vot::string>& suffixes){
//                    return std::ranges::any_of(suffixes, [&](const vot::string& suffix){
//                        return pt.ends_with(suffix);
//                    });
//                };
//
//                vot::BasicInfoComponent basicInfoComponent{};
//                vot::VertexDataComponent vertexDataComponent{};
//                vot::RenderComponent renderComponent{};
//                vot::AnimationComponent animationComponent{};
//                vot::RayTracingComponent rayTracingComponent{};
//
//                if (check({".vmd"})) {
//                    yic::resourceSystem->loader()->vmd(pt);
//                } else {
//                    if (check({".pmx"})){
//                        yic::resourceSystem->loader()->model(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent, true);
//                    } else if (check({".obj", ".fbx", ".gltf", ".glb"})) {
//                       yic::resourceSystem->loader()->model(pt, basicInfoComponent, vertexDataComponent, renderComponent, animationComponent, rayTracingComponent);
//                    }
//
//                    yic::systemHub.publishPolling(ev::tModelLoaded{});
//
//                    {
//                        _readyA.wait(false);
//                    }
//
//                    auto& e = ecs.entity(basicInfoComponent.name.data())
//                            .set(basicInfoComponent)
//                            .set(vertexDataComponent)
//                            .set(renderComponent)
//                            .set(animationComponent)
//                            .set(rayTracingComponent);
//                    if (vertexDataComponent.isMMD)
//                        e.add<vot::MMDTag>();
//
//                    {
//                        _doneA.store(true, std::memory_order_release);
//                        _doneA.notify_one();
//                    }
//
//                }
//            }
//        });
//
//        yic::systemHub.subscribePolling([&](const ev::tModelLoaded&){
//            {
//                _readyA.store(true, std::memory_order_release);
//                _readyA.notify_one();
//            }
//
//            {
//                _doneA.wait(false);
//            }
//
//            yic::logger->warn("begin");
//            auto e = ecs.entity(GLOBAL::pickON.c_str());
//            yic::sceneSystem->reloadTlas();
//
//            if(!e.get<vot::VertexDataComponent>()->isMMD){
//
//            e.get_mut<vot::RenderComponent>()->dsHandle = yic::desSystem->allocUpdateDescriptorSets([&]{
//                vot::DescriptorLayout2 layout{};
//
//                for(const auto& img : e.get<vot::RenderComponent>()->diffuseTextures){
//                    layout.emplace(vot::DescriptorLayout2::_1d{
//                            img->imageInfo(),
//                            e.get<vot::AnimationComponent>()->boneMatBuffer->bufferInfo(),
//                    });
//                }
//
//                return layout;
//            }, yic::renderLibrary->GP_Basic);
//            };
//            yic::logger->warn("end");
//
//            e.add<vot::RenderVisibleTag>();
//        });
//    }
} // sc