//
// Created by lenovo on 7/14/2024.
//

#include "RenderProcess.h"

#include <utility>

namespace yic {

    RenderProcess::RenderProcess(std::string id) : mId(std::move(id)){
        ct = EventBus::Get::vkSetupContext();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);

        mRenderSession = std::make_unique<RenderSession>(mId, ct.qIndexGraphicsPrimary_v(), rt.imageCount_v());
        
        mRenderGroupGraphics = RenderGroupGraphics ::configure(FrameRender::eColorDepthStencilRenderPass)
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addShader_("post_v.vert", vk::ShaderStageFlagBits::eVertex)
                ->addShader_("post_f.frag", vk::ShaderStageFlagBits::eFragment)
                ->build();
        mPostDescriptor = std::make_shared<Descriptor>(mId, *mRenderGroupGraphics);
    }

    RenderProcess::~RenderProcess() = default;


    auto RenderProcess::process() -> std::optional<vk::CommandBuffer>{
        if (mOffImage == nullptr){
            return std::nullopt;
        }
        auto index = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        auto& cmd = mRenderSession->beginCommandBuf(mExtent);
       // if (index == 1) {
            for (auto &rec: mCommandBufferRecordExtras) {
//                CommandBufferCoordinator::cmdDrawPrimary([&](vk::CommandBuffer& cmd){
                    rec(cmd);
//                });
            }
    //    }

        {
//            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
//                    FrameRender::eColorDepthStencilRenderPass,
//                    {mOffImage->framebuffers.begin() + offImageType::eStandard * mImageCount,
//                     mOffImage->framebuffers.begin() + offImageType::eStandard * mImageCount + mImageCount},
//                    RenderSession::ClearValue::ColorDepth()
//            });
            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                    FrameRender::eColorDepthStencilRenderPass,
                    {mOffImage->framebuffers.begin() + offImageType::eStandard * mImageCount,
                     mOffImage->framebuffers.begin() + offImageType::eStandard * mImageCount + mImageCount},
                    vk::SubpassContents::eSecondaryCommandBuffers,
                    RenderSession::ClearValue::ColorDepth()
            });

            for (auto &rec: mCommandBufferRecords) {
                rec(cmd);
            }

            mRenderSession->endRenderPass();
        }
        {
            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                    FrameRender::eColorDepthStencilRenderPass,
                    {mOffImage->framebuffers.begin() + offImageType::eComposite * mImageCount,
                     mOffImage->framebuffers.begin() + offImageType::eComposite * mImageCount + mImageCount},
                    RenderSession::ClearValue::ColorDepth()
            });
            mRenderGroupGraphics->render(cmd)
                    ->bindPipeline();
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                   mPostDescriptor->getDescriptorSets()[offImageType::eStandard * mImageCount + index],
                                   nullptr);
            cmd.draw(3, 1, 0, 0);

            mRenderSession->endRenderPass();
        }

        mRenderSession->endCommandBuf();

        return cmd;
    }

    auto RenderProcess::appendCommandRecord(const RenderProcess::fn_cmdRecord &record) -> void {
        mCommandBufferRecords.emplace_back(record);
    }
    auto RenderProcess::appendCommandRecordExtra(const RenderProcess::fn_cmdRecord &record) -> void {
        mCommandBufferRecordExtras.emplace_back(record);
    }

    auto RenderProcess::prepare() -> void {
        EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext& uiWidgetContext){
            mExtent.store(vk::Extent2D{(uint32_t)uiWidgetContext.viewportSize.value().x, (uint32_t )uiWidgetContext.viewport_height_v()});
            ct.qGraphicsPrimary_ref().waitIdle();
            mImageCount = rt.imageCount_v();

            mOffImage = Allocator::allocImgOffScreen_DepthStencilAndFramebuffers(vkImageConfig{mExtent}, FrameRender::eColorDepthStencilRenderPass, mId, mImageCount * offImageType::eCount);

            ImGuiDescriptorManager::updateImage(mId, {mOffImage->imageViews.begin() + (offImageType::eComposite * mImageCount),
                                                      mOffImage->imageViews.begin() + (offImageType::eComposite * mImageCount + mImageCount)});

            mPostDescriptor->updateDesSet(offImageType::eComposite * mImageCount, {ImgInfo{
                    std::vector<vk::ImageView>{mOffImage->imageViews.begin(),
                                               mOffImage->imageViews.begin() +
                                               offImageType::eComposite * mImageCount}}});
        }, mId);
        mCommandBufferRecords.clear();
        mCommandBufferRecordExtras.clear();
    }














//    auto RenderProcess::registrationProcess(const std::string& name, const uint32_t& framebufferOffset) -> void {
//        mRegisters.emplace_back(name, framebufferOffset);
//    }
//
//    auto RenderProcess::process() -> std::optional<vk::CommandBuffer>{
//        if (mOffImage == nullptr){
//            return std::nullopt;
//        }
//        auto& cmd = mRenderSession->beginCommandBuf(mExtent);
//
//        for(const auto& reg : mRegisters){
//            mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
//                FrameRender::eColorDepthStencilRenderPass,
//                {mOffImage->framebuffers.begin() + reg.second * mImageCount,
//                 mOffImage->framebuffers.begin() + reg.second * mImageCount + mImageCount},
//                 RenderSession::ClearValue::ColorDepth()
//            });
//
//            for(const auto& rec : mCommandBufRecords[reg.first]){
//                rec(cmd);
//            }
//
//            mRenderSession->endRenderPass();
//        }
//
//        return cmd;
//    }
//
//    auto RenderProcess::appendCommandRec(const std::string& name, const fn_cmdRecord &rec) -> void {
//        mCommandBufRecords[name].emplace_back(rec);
//    }
//
//    auto RenderProcess::prepare() -> void {
//        EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext& uiWidgetContext){
//            mExtent.store(vk::Extent2D{(uint32_t)uiWidgetContext.viewportSize.value().x, (uint32_t )uiWidgetContext.viewport_height_v()});
//            ct.qGraphicsPrimary_ref().waitIdle();
//            mImageCount = rt.imageCount_v();
//
//            mOffImage = Allocator::allocImgOffScreen_DepthStencilAndFramebuffers(vkImageConfig{mExtent}, FrameRender::eColorDepthStencilRenderPass, mId, mImageCount * mProcessCount);
//
//            ImGuiDescriptorManager::updateImage(mId, {mOffImage->imageViews.begin() + (offImageType::eComposite * mImageCount),
//                                                      mOffImage->imageViews.begin() + (offImageType::eComposite * mImageCount + mImageCount)});
//
//            mPostDescriptor->updateDesSet(offImageType::eComposite * mImageCount, {ImgInfo{
//                    std::vector<vk::ImageView>{mOffImage->imageViews.begin(),
//                                               mOffImage->imageViews.begin() +
//                                               offImageType::eComposite * mImageCount}}});
//        }, mId);
//        mCommandBufferRecords.clear();
//    }


} // yic