//
// Created by lenovo on 8/31/2024.
//

#include "RenderProcess.h"

#include <utility>

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/RHI/FrameRender.h"
#include "Engine/RHI/Allocator.h"


namespace yic {

    RenderProcess::RenderProcess(std::string id) : mId(std::move(id)){
        ct = mg::SystemHub.val<ev::pVkSetupContext>();
        rt = mg::SystemHub.val<ev::hVkRenderContext>(toolkit::enum_name(RenderPhase::ePrimary));

        mImageCount = rt.framebuffers->size();
        mImageIndex = rt.activeImageIndex;
        mRenderSession = std::make_unique<RenderSession>(mId, ct.queueFamily->gIndexPrimary(), mImageCount, vk::CommandBufferUsageFlagBits::eSimultaneousUse);

        mOffImage = mg::Allocator->allocImage(ImageConfig()
                .setExtent(mExtent)
                .setImageFlags(ImageFlags::eDepthStencil)
                .setRenderPass(FrameRender::eColorDepthStencilRenderPass)
                .setImageCount(mImageCount), mId);

        ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);

        renderTargetChange();
    }

    RenderProcess::~RenderProcess() = default;

    auto RenderProcess::appendRenderPassProcessSecondaryCommand(const uint8_t& seq, const std::shared_ptr<vkImage> &imgSptr, const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecs[seq] = [this, ptr = imgSptr.get(), rec](vk::CommandBuffer &cmd) {
            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, ptr->framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::clearValue::colorDepth
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        };
    }

    auto RenderProcess::appendRenderPassProcessSecondaryCommand(const uint8_t& seq, const vkImage* imgPtr, const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecs[seq] = [this, imgPtr, rec](vk::CommandBuffer &cmd) {
            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, imgPtr->framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::clearValue::colorDepth
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        };
    }

    auto RenderProcess::appendRenderPassProcessCommand(const uint8_t &seq, const std::vector<vk::Framebuffer>& framebuffers, const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecs[seq] = [=, this](vk::CommandBuffer& cmd) {

            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::clearValue::colorDepth
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        };
    }

    auto RenderProcess::appendRenderPassProcessCommand(const uint8_t &seq, const vkImage_sptr& imageSptr, const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecs[seq] = [this, ptr = imageSptr.get(), rec](vk::CommandBuffer& cmd) {

            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, ptr->framebuffers,
                    vk::SubpassContents::eSecondaryCommandBuffers, RenderSession::clearValue::colorDepth
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        };
    }

    auto
    RenderProcess::appendProcessCommand(const uint8_t &seq, const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecs[seq] = rec;
    }

    auto RenderProcess::appendFinalProcessCommand(const uint8_t &seq) -> void {
        mCommandBufRecs.resize(seq + 1);
        mCommandBufRecs[seq] = [=, this](vk::CommandBuffer& cmd){

            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, mOffImage->framebuffers,
                    RenderSession::clearValue::colorDepth
            });

            if (!mDescriptor->getDescriptorSets().empty()) {
                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->acquire());
                cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                       mDescriptor->getDescriptorSets()[*mImageIndex], nullptr);
                cmd.draw(3, 1, 0, 0);
            }

            mRenderSession->endRenderPass();
        };
    }


    auto RenderProcess::process() -> std::optional<vk::CommandBuffer>{
        if (mOffImage == nullptr){
            return std::nullopt;
        }

        auto& cmd = mRenderSession->beginCommandBuf(mExtent);

        for(const auto& fn : mCommandBufRecsPrepose){
            fn(cmd);
        }

        for(const auto& fn : mCommandBufRecs){
            fn(cmd);
        }

        mRenderSession->endCommandBuf();

        return cmd;
    }

    //auto RenderProcess::prepare() -> void {
//        EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext& uiWidgetContext){
//            mExtent = vk::Extent2D{(uint32_t)uiWidgetContext.viewportSize.value().x, (uint32_t )uiWidgetContext.viewport_height_v()};
//            ct.queueFamily->gPrimary().waitIdle();
//
//            mOffImage = mg::Allocator->allocImage(ImageConfig()
//                                                          .setExtent(mExtent)
//                                                          .setImageFlags(ImageFlags::eDepthStencil)
//                                                          .setRenderPass(FrameRender::eColorDepthStencilRenderPass)
//                                                          .setImageCount(mImageCount), mId);
//
//            ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);
//        }, mId);

//    }

//    auto RenderProcess::updateDescriptor(const uint8_t &seq, const std::shared_ptr<vkImage> &image) -> void {
//        if (mDyRenderTargetStageImgs.size() <= seq){
//            mDyRenderTargetStageImgs.resize(seq + 1);
//        }
//        mDyRenderTargetStageImgs[seq] = image;
//        std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> temp;
//
//        for(const auto& img : mDyRenderTargetStageImgs){
//            if (img != nullptr)
//                temp.emplace_back(ImgInfo{img});
//        }
//        mDescriptor->updateDesSet(mImageCount, temp);
//    }

    auto RenderProcess::rebuild() -> void {
        mCommandBufRecs.clear();
    }

    auto RenderProcess::renderTargetChange() -> void {
        mg::SystemHub.subscribe([&](const ev::eRenderTargetSizeChange& sizeChange){
            mExtent = vk::Extent2D{(uint32_t)sizeChange.size.x, (uint32_t )sizeChange.size.y};
            ct.queueFamily->gPrimary().waitIdle();

            mOffImage = mg::Allocator->allocImage(ImageConfig()
                                                          .setExtent(mExtent)
                                                          .setImageFlags(ImageFlags::eDepthStencil)
                                                          .setRenderPass(FrameRender::eColorDepthStencilRenderPass)
                                                          .setImageCount(mImageCount), mId);

            ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);
        });
    }

    auto RenderProcess::updateDescriptorImpl(const uint8_t &seq,
                                         const std::variant<vkImage_sptr, yic2::Image_sptr> &image) -> void {
        if (mDynamicRenderTargetStageImages.size() <= seq){
            mDynamicRenderTargetStageImages.resize(seq + 1);
        }
        mDynamicRenderTargetStageImages[seq] = image;
        std::vector<std::variant<ImgInfo, BufInfo, AccelInfo>> temp;

        for(const auto& stageImage : mDynamicRenderTargetStageImages){
            std::visit([&](auto &&arg) {
                if (arg != nullptr)
                    temp.emplace_back(ImageInfo{arg});
            }, stageImage);
        }

        mDescriptor->updateDesSet(mImageCount, temp);
    }

    auto RenderProcess::appendRenderPassProcessCommandPrepose(const yic2::Image_sptr &imageSptr,
                                                              const yic::RenderProcess::recCommandFn &rec) -> void {
        mCommandBufRecsPrepose.emplace_back() = [this, ptr = imageSptr.get(), rec](vk::CommandBuffer& cmd) {

            mRenderSession->beginRenderPass(RenderSession::passInfo{
                    FrameRender::eColorDepthStencilRenderPass, std::vector<vk::Framebuffer>(ptr->framebuffers.begin(), ptr->framebuffers.end()),
                    vk::SubpassContents::eInline, RenderSession::clearValue::colorDepth
            });

            rec(cmd);

            mRenderSession->endRenderPass();
        };
    }

} // yic