//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessManager.h"

namespace yic {

    RenderProcessManager::RenderProcessManager() {
        FrameRender::get();
        //mRenderProcess[enum_name(RenderProcessPhases::ePrimary)] = std::make_unique<RenderProcess>(enum_name(RenderProcessPhases::ePrimary));
        init();

        mEcsManager = std::make_unique<sc::ECSManager>();
    }

    RenderProcessManager::~RenderProcessManager() = default;

    //---------------------------------------------------------------------------------------------//
    ///                                     (〃'▽'〃)
    //---------------------------------------------------------------------------------------------//

    auto RenderProcessManager::drawBuild() -> void {
//        auto& priRp = mRenderProcess[enum_name(RenderProcessPhases::ePrimary)];
//        priRp->appendCommandRecord([&](vk::CommandBuffer& cmd){
//            mEcsManager->Render(cmd);
//        });
//        priRp->appendCommandRecordExtra([&](vk::CommandBuffer& cmd){
//            mEcsManager->renderStorage(cmd);
//        });
    }

    //---------------------------------------------------------------------------------------------//
    ///                                    ∠( °ω°)／
    //---------------------------------------------------------------------------------------------//

    auto RenderProcessManager::RenderProcedure_impl() -> std::vector<vk::CommandBuffer> {
//        cmds.clear();
//
//        for(auto& key : {RenderProcessPhases::ePrimary}){
//            auto it = mRenderProcess.find(enum_name(key));
//            if (it != mRenderProcess.end()){
//                if (it->second->process().has_value())
//                    cmds.emplace_back(it->second->process().value());
//            }
//        }
//
//        return {cmds.begin(), cmds.end()};
        return  renderProcedure();
    }

    auto RenderProcessManager::prepare_impl() -> void {
//        sc::globalCamera.computeViewProjMatrix();
//
//        for(const auto& it : mRenderProcess){
//            it.second->prepare();
//        }
//
//        vk::CommandBuffer cmd;

        Prepare();
        mEcsManager->prepare();
//        mEcsManager->Render(cmd);
//        mEcsManager->renderStorage(cmd);
    }

    auto RenderProcessManager::clear_impl() -> void {
        mRenderProcess.clear();
        mEcsManager->clear();
    }


    ///////////////------------------------------------

    auto RenderProcessManager::init() -> void {
        auto primary = registerRenderProcess(RenderProcessPhases::ePrimary);
        primary ->acquire() = RenderGroupGraphics ::configure(FrameRender::eColorDepthStencilRenderPass)
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addShader_("post_v.vert", vk::ShaderStageFlagBits::eVertex)
                ->addShader_("post_f.frag", vk::ShaderStageFlagBits::eFragment)
                ->build();
        primary->descriptor() = Descriptor::configure(*primary->acquire());
    }

    auto RenderProcessManager::registerRenderProcess(RenderProcessPhases phases) -> RenderProcessT* {
        auto& rp = mRenderProcessT[static_cast<int>(phases)] = std::make_unique<RenderProcessT>(enum_name(phases));
        EventBus::update(et::pRenderProcess{rp.get()}, enum_name(phases));
        return rp.get();
    }

    auto RenderProcessManager::Prepare() -> void {
        sc::globalCamera.computeViewProjMatrix();

        for(const auto& it : mRenderProcessT){
            it.second->prepare();
        }

        mRenderProcessT[static_cast<int>(RenderProcessPhases::ePrimary)]->appendFinalProcessCommand({});
    }

    auto RenderProcessManager::renderProcedure() -> std::vector<vk::CommandBuffer> {
        cmds.clear();

        for(const auto& [key, uptr] : mRenderProcessT){
            if (uptr->process().has_value())
                cmds.emplace_back(uptr->process().value());
        }

        return {cmds.begin(), cmds.end()};
    }


} // yic