//
// Created by lenovo on 7/14/2024.
//

#include "RenderProcess.h"

#include <utility>

namespace yic {

    RenderProcess::RenderProcess(std::string id) : mId(std::move(id)){
        ct = EventBus::Get::vkSetupContext();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);
        FrameRender::get();

        mRenderSession = std::make_unique<RenderSession>(mId, ct.qIndexGraphicsPrimary_v(), rt.imageCount_v());
    }

    RenderProcess::~RenderProcess() = default;


    auto RenderProcess::process() -> std::optional<vk::CommandBuffer>{
        if (mOffImage == nullptr){
            return std::nullopt;
        }
        auto& cmd = mRenderSession->beginCommandBuf(mExtent);

        mRenderSession->beginRenderPass(RenderSession::RenderPassInfo{
                FrameRender::eColorDepthStencilRenderPass, mOffImage->framebuffers, RenderSession::ClearValue::ColorDepth()
        });

        for(auto& rec : mCommandBufferRecords){
            rec(cmd);
        }

        mRenderSession->endRenderPass();
        mRenderSession->endCommandBuf();

        return cmd;
    }

    auto RenderProcess::appendCommandRecord(const RenderProcess::fn_cmdRecord &record) -> void {
        mCommandBufferRecords.emplace_back(record);
    }

    auto RenderProcess::prepare() -> void {
        EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext& uiWidgetContext){
            mExtent.store(vk::Extent2D{(uint32_t)uiWidgetContext.viewportSize.value().x, (uint32_t )uiWidgetContext.viewport_height_v()});
            EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
            mOffImage = Allocator::allocImgOffScreen_DepthStencilAndFramebuffers(vkImageConfig{{mExtent}}, FrameRender::eColorDepthStencilRenderPass, mId, rt.imageCount_v());
            ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);
        }, mId);
        mCommandBufferRecords.clear();
    }


} // yic