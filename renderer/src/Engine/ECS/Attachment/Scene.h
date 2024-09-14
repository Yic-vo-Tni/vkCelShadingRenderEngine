//
// Created by lenovo on 8/30/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

#include <utility>

#include "Engine/RHI/vkAsset.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/ECS/Camera/Camera.h"
#include "Engine/RHI/Descriptor.h"
#include "Engine/RHI/RenderGroup.h"
#include "Engine/RHI/RenderProcess.h"

namespace sc {

    struct Scene {
        explicit Scene(std::string id) : id(std::move(id)) {};

        std::string id;
        std::shared_ptr<yic::vkAccel> tlas;
        std::shared_ptr<yic::Descriptor> rtDesSet;
        std::vector<yic::vkAccel_sptr> blass;
        vot::vector<sc::MeshBufAddress> meshBufferAddresses;
        yic::vkBuffer_sptr  meshBufferAddressBuffer;
        bool update = false;
    };

    class SceneManager{
    public:
        SceneManager();
        vkGet auto make = []{ return Singleton<SceneManager>::get(); };
        auto switchScene(const std::string& id) -> void;
        auto loadScene(const std::string& id = {}) -> void;
        auto unloadScene(const std::string& id) -> void;
        auto addModel(const Model* model) -> void;
        auto updateScene() -> void;
        auto removeModel(const std::string& id) -> void;

    public:
        auto cBlas(Model* model) -> void;
        auto getActiveScene() { return mActiveScene; }
    private:
        auto buildTLAS() -> void;
        auto cRTRenderGroup() -> void;
        auto render() -> void;
    private:
        vk::Device mDevice;
        vk::DispatchLoaderDynamic mDyDispatcher;

        uint8_t mSceneCount{0};
        Scene* mActiveScene{};
        vk::Extent2D mExtent{2560, 1440};
//        yic::RenderProcess* mRenderHandle;
        yic::RenderProcessHanlde mRenderHandle;
        yic::RenderGroupRayTracing_sptr mRayTracingGroup;
        yic::Descriptor_sptr mRTDescriptor;
        yic2::Image_sptr mRTOffImage;
        yic2::Image_sptr mRenderTargetOffImage;

        std::unordered_map<std::string, Scene> mScenes;
        oneapi::tbb::spin_rw_mutex mRwMutex;

    };

} // sc

namespace mg{
    inline sc::SceneManager* SceneManager;
}

#endif //VKCELSHADINGRENDERER_SCENE_H
