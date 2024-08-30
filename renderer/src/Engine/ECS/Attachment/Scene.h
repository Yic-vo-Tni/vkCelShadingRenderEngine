//
// Created by lenovo on 8/30/2024.
//

#ifndef VKCELSHADINGRENDERER_SCENE_H
#define VKCELSHADINGRENDERER_SCENE_H

#include "Engine/RHI/vkAsset.h"
#include "Engine/ECS/Model/ModelStruct.h"
#include "Engine/RHI/Descriptor.h"

namespace sc {

    class Scene {
    public:
        explicit Scene(const std::string& id = IdGenerator::uniqueId());
        ~Scene() = default;
        auto updateScene(const Model& model) -> void;
        auto removeModel(const std::string& id) -> void;
        [[nodiscard]] auto getSceneId() const -> std::string { return mId; };

    private:
        std::string mId;
        std::shared_ptr<yic::vkAccel> mTLAS;
        std::shared_ptr<yic::Descriptor> mRtDes;
        std::vector<const Model*> mModels;
    };

} // sc

#endif //VKCELSHADINGRENDERER_SCENE_H
