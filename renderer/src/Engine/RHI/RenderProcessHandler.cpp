//
// Created by lenovo on 7/17/2024.
//

#include "RenderProcessHandler.h"

namespace yic {

    RenderProcessHandler::RenderProcessHandler() {
        FrameRender::get();

        mRenderProcess.resize(static_cast<int>(RenderPhase::eCount));

        auto primary = registerRenderProcess(RenderPhase::ePrimary);
        primary ->acquire() = RenderGroupGraphics ::configure(FrameRender::eColorDepthStencilRenderPass)
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addDesSetLayout_(0, 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addShader_("post_v.vert", vk::ShaderStageFlagBits::eVertex)
                ->addShader_("post_f.frag", vk::ShaderStageFlagBits::eFragment)
                ->build();
        primary->descriptor() = Descriptor::configure(*primary->acquire());
        primary->appendFinalProcessCommand(PrimaryRenderSeq::eCount);

        mEcsManager = std::make_unique<sc::ECSManager>();
    }

    RenderProcessHandler::~RenderProcessHandler() = default;

    auto RenderProcessHandler::prepare() -> void {
        sc::globalCamera.computeViewProjMatrix();

//        for(const auto& rp : mRenderProcess){
//            rp->prepare();
//        }

        mEcsManager->prepare();
    }

    auto RenderProcessHandler::procedure() -> std::vector<vk::CommandBuffer> {
        cmds.clear();

        for(const auto& rp : mRenderProcess){
            auto r = rp->process();
            if (r.has_value()){
                cmds.emplace_back(r.value());
            }
        }

        return cmds;
    }

    auto RenderProcessHandler::clear() -> void {
        mRenderProcess.clear();
        mEcsManager->clear();
    }


    auto RenderProcessHandler::registerRenderProcess(RenderPhase phases) -> RenderProcess* {
        auto& rp = mRenderProcess[static_cast<int>(phases)] = std::make_unique<RenderProcess>(enum_name(phases));
        mg::SystemHub.sto(ev::pRenderProcess{rp.get()}, toolkit::enum_name(phases));
        return rp.get();
    }



} // yic