//
// Created by lenovo on 8/30/2024.
//

#include "Scene.h"
#include "Engine/RHI/Allocator.h"

namespace sc {


    Scene::Scene(const std::string& id) {
        mId = id;
    }

    auto Scene::updateScene(const Model &model) -> void {
        mModels.emplace_back(&model);

//        mTLAS = yic::Allocator::updateAccel(model.as.blas);
//        std::vector<vk::Buffer> bufs;
//        for(auto& m : mModels){
//            bufs.emplace_back(m->mesh.vertBuf->buffer);
//            mRtDes->updateDesSetAuto(bufs);
//        }

    }

    auto Scene::removeModel(const std::string &id) -> void {
        auto remove = std::ranges::remove_if(mModels, [&id](const Model*& model){
           return id == model->info.id;
        });
        mModels.erase(remove.begin(), remove.end());
    }

} // sc