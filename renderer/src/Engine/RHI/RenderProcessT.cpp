//
// Created by lenovo on 8/31/2024.
//

#include "RenderProcessT.h"

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/RHI/Allocator.h"


namespace yic {

    RenderProcessT::RenderProcessT(std::string id) : mId(std::move(id)){
        ct = EventBus::Get::vkSetupContext();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);

        mRenderSession = std::make_unique<RenderSession>(mId, ct.qIndexGraphicsPrimary_v(), rt.imageCount_v());
    }

    RenderProcessT::~RenderProcessT() = default;

    auto RenderProcessT::appendRenderPassProcessSecondaryCommand(const uint8_t& seq,
            const std::vector<vk::Framebuffer> &framebuffers, const yic::RenderProcessT::recCommandFn &rec) -> void {

        mCommandBufRecs.emplace(seq, [this, framebuffers, rec](vk::CommandBuffer& cmd) {
            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                    FrameRender::eColorDepthStencilRenderPass, framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::ClearValue::ColorDepth()
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        });
    }

    auto RenderProcessT::appendRenderPassProcessCommand(const uint8_t &seq, const std::vector<vk::Framebuffer>& framebuffers, const yic::RenderProcessT::recCommandFn &rec) -> void {
        mCommandBufRecs.emplace(seq, [=, this](vk::CommandBuffer& cmd) {

            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                    FrameRender::eColorDepthStencilRenderPass, framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::ClearValue::ColorDepth()
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        });
    }

    auto
    RenderProcessT::appendProcessCommand(const uint8_t &seq, const yic::RenderProcessT::recCommandFn &rec) -> void {
        mCommandBufRecs.emplace(seq, rec);
    }

    auto RenderProcessT::appendFinalProcessCommand(const yic::RenderProcessT::recCommandFn &rec) -> void {
        mCommandBufRecs.emplace(UINT8_MAX, [=, this](vk::CommandBuffer& cmd){
            auto index = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                FrameRender::eColorDepthStencilRenderPass, mOffImage->framebuffers,
                RenderSession::ClearValue::ColorDepth()
            });

            if (!mDescriptor->getDescriptorSets().empty()) {
                mRenderGroupGraphics->render(cmd)
                        ->bindPipeline();
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                       mDescriptor->getDescriptorSets()[index], nullptr);
                cmd.draw(3, 1, 0, 0);
            }

            mRenderSession->endRenderPass();
        });
    }


    auto RenderProcessT::process() -> std::optional<vk::CommandBuffer>{
        if (mOffImage == nullptr){
            return std::nullopt;
        }

        auto& cmd = mRenderSession->beginCommandBuf(mExtent);
        for(const auto& [seq, fn] : mCommandBufRecs){
            fn(cmd);
        }

        mRenderSession->endCommandBuf();

        return cmd;
    }

    auto RenderProcessT::prepare() -> void {
        EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext& uiWidgetContext){
            mExtent.store(vk::Extent2D{(uint32_t)uiWidgetContext.viewportSize.value().x, (uint32_t )uiWidgetContext.viewport_height_v()});
            ct.qGraphicsPrimary_ref().waitIdle();
            mImageCount = rt.imageCount_v();

            mOffImage = Allocator::allocImgOffScreen_DepthStencilAndFramebuffers(vkImageConfig{mExtent}, FrameRender::eColorDepthStencilRenderPass, mId, mImageCount);

            ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);
        }, mId);

        mCommandBufRecs.clear();
    }

    auto RenderProcessT::updateDescriptor(const uint8_t &seq, const std::shared_ptr<vkImage> &image) -> void {
        mDynamicRenderTargetStageImages.emplace(seq, image);
        std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> temp;
        for(const auto& [key, img] : mDynamicRenderTargetStageImages){
            temp.emplace_back(ImgInfo{img});
        }
        mDescriptor->updateDesSet(rt.imageCount_v(), temp);
    }

} // yic