//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessManager.h"

namespace yic {

    RenderProcessManager::RenderProcessManager() {
        FrameRender::get();
        mRenderProcess[enum_name(RenderProcessPhases::ePrimary)] = std::make_unique<RenderProcess>(enum_name(RenderProcessPhases::ePrimary));
        mEcsManager = std::make_unique<sc::ECSManager>();
    }

    RenderProcessManager::~RenderProcessManager() = default;

    //---------------------------------------------------------------------------------------------//
    ///                                     (〃'▽'〃)
    //---------------------------------------------------------------------------------------------//

    auto RenderProcessManager::drawBuild() -> void {
        auto& priRp = mRenderProcess[enum_name(RenderProcessPhases::ePrimary)];
        priRp->appendCommandRecord([&](vk::CommandBuffer& cmd){
            mEcsManager->Render(cmd);
        });
        priRp->appendCommandRecordExtra([&](vk::CommandBuffer& cmd){
            mEcsManager->renderStorage(cmd);
        });
    }

    //---------------------------------------------------------------------------------------------//
    ///                                    ∠( °ω°)／
    //---------------------------------------------------------------------------------------------//

    auto RenderProcessManager::RenderProcedure_impl() -> std::vector<vk::CommandBuffer> {
        cmds.clear();

        for(auto& key : {RenderProcessPhases::ePrimary}){
            auto it = mRenderProcess.find(enum_name(key));
            if (it != mRenderProcess.end()){
                if (it->second->process().has_value())
                    cmds.emplace_back(it->second->process().value());
            }
        }

        return {cmds.begin(), cmds.end()};
    }

    auto RenderProcessManager::prepare_impl() -> void {
        sc::globalCamera.computeViewProjMatrix();

//        tbb::parallel_for_each(mRenderProcess.begin(), mRenderProcess.end(), [](auto& it){
//            if (it.second){ it.second->prepare(); }
//        });
        for(const auto& it : mRenderProcess){
            it.second->prepare();
        }

        drawBuild();
    }

    auto RenderProcessManager::clear_impl() -> void {
        mRenderProcess.clear();
        mEcsManager->clear();
    }


} // yic