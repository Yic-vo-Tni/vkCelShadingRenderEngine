//
// Created by lenovo on 7/31/2024.
//

#include "RenderSession.h"

namespace yic {

    RenderSession::RenderSession(std::string id, const uint32_t &qIndex, const uint32_t& CommandBufferCount)
            : mId(std::move(id)),
              mQueueIndex(qIndex),
              mDevice(*mg::SystemHub.val<ev::pVkSetupContext>().device),
              mFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit),
              mCommandPool(createCommandPool()),
              mCommandBuffers(createCommandBuffers(CommandBufferCount)){

    }
    RenderSession::RenderSession(std::string id, const uint32_t &qIndex, const uint32_t &CommandBufferCount,
                                 vk::CommandBufferUsageFlags flags) : mId(std::move(id)),
                                                                     mQueueIndex(qIndex),
                                                                     mDevice(*mg::SystemHub.val<ev::pVkSetupContext>().device),
                                                                     mFlags(flags),
                                                                     mCommandPool(createCommandPool()),
                                                                     mCommandBuffers(createCommandBuffers(CommandBufferCount)){

    }

    RenderSession::~RenderSession() {
        mDevice.destroy(mCommandPool);
    }

    auto RenderSession::createCommandPool() -> vk::CommandPool {
        Rvk_y("create cmd pool") = [&] {
            return mDevice.createCommandPool(vk::CommandPoolCreateInfo()
                                                     .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                                     .setQueueFamilyIndex(mQueueIndex));
        };
    }

    auto RenderSession::createCommandBuffers(uint32_t cmdCount) -> std::vector<vk::CommandBuffer> {
        Rvk_y("create cmd") = [&]{
            return mDevice.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo().setCommandPool(mCommandPool)
                            .setCommandBufferCount(cmdCount)
                            .setLevel(vk::CommandBufferLevel::ePrimary));
        };
    }

    auto RenderSession::beginCommandBuf(vk::Extent2D extent2D) -> vk::CommandBuffer & {
        vk::CommandBufferBeginInfo beginInfo{mFlags};

        auto imageIndex = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        mActiveCommandBuffer = mCommandBuffers[imageIndex];

        EventBus::update(et::vkCommandBuffer{.cmd = mActiveCommandBuffer}, mId);
        mActiveCommandBuffer.begin(beginInfo);

        vk::Viewport viewport{
                0.f, 0.f,
                static_cast<float>(extent2D.width), static_cast<float>(extent2D.height),
                0.f, 1.f
        };
        vk::Rect2D scissor{{0, 0}, extent2D};
        mActiveCommandBuffer.setViewport(0, viewport);
        mActiveCommandBuffer.setScissor(0, scissor);

        mExtent = extent2D;

        return mActiveCommandBuffer;
    }

    auto RenderSession::endCommandBuf() -> void {
        mActiveCommandBuffer.end();
    }

    auto RenderSession::endRenderPass() -> void {
        mActiveCommandBuffer.endRenderPass();
    }

    auto RenderSession::beginRenderPass(RenderSession::passInfo info) -> void {
        auto imageIndex = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        if (!info.extent.has_value()){
            info.extent = mExtent;
        }
        vk::RenderPassBeginInfo renderPassBeginInfo{info.renderPass, info.framebuffers[imageIndex],
                                                    {info.offset2D, info.extent.value()}, info.clearValues};

        mActiveCommandBuffer.beginRenderPass(renderPassBeginInfo, info.subpassContents);
    }


} // yic