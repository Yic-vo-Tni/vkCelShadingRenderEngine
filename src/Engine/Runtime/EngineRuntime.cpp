//
// Created by lenovo on 10/14/2024.
//

#include "EngineRuntime.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
#include "Camera/Camera.h"
#include "RHI/DescriptorSystem.h"
#include "RS/ResourceSystem.h"
#include "SM/Scene.h"
#include "System/RenderLibrary.h"
#include "Editor/ShaderHotReload/ShaderHotReload.h"

#include "oneapi/tbb/tick_count.h"


namespace sc {

    EngineRuntime::EngineRuntime() {
        vot::EntityView<vot::bit::eEntityViewDefault>::Init(ecs); // TODO: if more scene(entt::register)
        prepose();
        buildSet0();
    }

    EngineRuntime::~EngineRuntime() {
        yic::logger->warn("~ ecs");

        ct.device->waitIdle();

        yic::systemHub.pub(ev::tDestroyVMA{});
        yic::systemHub.setEvent(ev::tModelLoaded{});
        yic::systemHub.pub(ev::tDestroyVMA{});

        RenderLibrary::destroy();
        rs::ResourceSystem::destroy();
        sm::SceneSystem::destroy();
    };

    auto EngineRuntime::prepose() -> void {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();

        yic::resourceSystem = rs::ResourceSystem::make(ecs);
        yic::sceneSystem = sm::SceneSystem::make(ecs);
        yic::renderLibrary = RenderLibrary::make();

        inspectorPanel = std::make_unique<InspectorPanel>(ecs);
        submissionSystem = std::make_unique<RenderSubmissionSystem>(ecs);
    }

    auto EngineRuntime::render() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded, ev::tDestroyEntity>();

        yic::indexRing.read([&] {
            yic::shaderHot->frame();
            inspectorPanel->frame();

            yic::sceneSystem->frame();
            submissionSystem->frame(); // TODO: Split into -> RHI thread
        }, vot::eFast, vot::eSlow);
    }

    auto EngineRuntime::fastLogic() -> void {
        yic::indexRing.write([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // TODO: Take on some takes in future

            Camera::UpdateUnique();
        }, vot::eFast);
    }

    auto EngineRuntime::slowLogic() -> void {
        yic::systemHub.dispatch<ev::tModelLoaded, ev::tDestroyEntity>();

        yic::indexRing.write([&] {
            yic::resourceSystem->frame(); // TODO: Split into -> Animation.frame()
        }, vot::eSlow);
    }

    auto EngineRuntime::buildSet0() -> void {
        auto& cam = GLOBAL::entity::camera.make_va<Camera>();
        cam.computeViewProjMatrix();

        vot::comp::Light::Meta meta{};
        meta.entries.resize(30); // HACK: dynamic increase
        meta.ssbo = yic::allocator->allocBuffer(sizeof(vot::comp::Light::Entry) * 30, vk::BufferUsageFlagBits::eStorageBuffer, "Light entries");
        meta.ssbo->update(meta.entries);

        vot::BasicInfoComponent basicInfoComponent{};
        vot::VertexDataComponent vertexDataComponent{};
        vot::RenderComponent renderComponent{};
        vot::AnimationComponent animationComponent{};

        yic::resourceSystem->mLoader->mAssimpLoader->Load(tex_path "../Model/Light/Sphere.gltf", basicInfoComponent, vertexDataComponent, renderComponent, animationComponent);
        // FIXME: tex_path -> mod_path

        vot::EntityView<vot::bit::eEntityViewDefault>::create()
            .mark<vot::mark::eVisible>()
            .emplace(meta, basicInfoComponent, vertexDataComponent, renderComponent);

        auto& [set0] = GLOBAL::entity::set0.make_va<vot::DescriptorSet0>();
        std::ranges::for_each(std::views::iota(0, 3), [&](auto i) -> void {
            set0[i] = yic::desSystem->allocUpdateDescriptorSets([&] {
                vot::DescriptorLayout2 layout2;
                layout2.emplace({cam.vpBufferInfo(i), meta.ssbo->bufferInfo()});
                return layout2;
            }, yic::renderLibrary->GP_Basic_Assimp, 0, 1);
        });
    }


    auto EngineRuntime::calFnTimeConsuming(const std::function<void()> &fn) -> void {
        const auto b = oneapi::tbb::tick_count::now();

        fn();

        yic::logger->info("consuming: {0}", (oneapi::tbb::tick_count::now() - b).seconds() * 1000.0);
    }


} // sc




