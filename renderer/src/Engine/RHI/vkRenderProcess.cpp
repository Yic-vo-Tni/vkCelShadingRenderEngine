//
// Created by lenovo on 7/14/2024.
//

#include "vkRenderProcess.h"

#include <utility>

namespace yic {

    vkRenderProcess::vkRenderProcess(vk::Extent2D extent, std::string id) : mId(std::move(id)), mExtent(extent) {
        ct = EventBus::Get::vkSetupContext();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);
        vkFrameRender::get();

        mCommand = std::make_unique<vkCommand>(mId, ct.qIndexGraphicsPrimary_v(), rt.imageCount_v());

        mOffImage = vkAllocator::allocImgOffScreen_DepthStencilAndFramebuffers(vkImageConfig{{mExtent}}, vkFrameRender::eColorDepthStencilRenderPass, mId, rt.imageCount_v());

        ImGuiDescriptorManager::updateImage(mId, mOffImage->imageViews);
    }

    vkRenderProcess::~vkRenderProcess() = default;


    auto vkRenderProcess::process() -> vk::CommandBuffer{
        auto& cmd = mCommand->beginCommandBuf(mExtent);
        mCommand->beginRenderPass(vkCommand::RenderPassInfo{
                vkFrameRender::eColorDepthStencilRenderPass, mOffImage->framebuffers, vkCommand::ClearValue::ColorDepth()
        });

        for(auto& rec : mCommandBufferRecords){
            rec(cmd);
        }

        mCommand->endRenderPass();
        mCommand->endCommandBuf();

        return cmd;
    }

    auto vkRenderProcess::appendCommandRecord(const vkRenderProcess::fn_cmdRecord &record) -> void {
        mCommandBufferRecords.emplace_back(record);
    }


} // yic