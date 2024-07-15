//
// Created by lenovo on 6/10/2024.
//

#include "vkCommand.h"

#include <utility>

namespace yic {

    vkCommand::vkCommand(std::string id, const uint32_t &qIndex, const uint32_t& CommandBufferCount)
            : mId(std::move(id)),
              mQueueIndex(qIndex),
              mDevice(EventBus::Get::vkSetupContext().device_ref()),
              mCommandPool(createCommandPool()),
              mCommandBuffers(createCommandBuffers(CommandBufferCount)) {

    }

    vkCommand::~vkCommand() {
        mDevice.destroy(mCommandPool);
    }

    auto vkCommand::createCommandPool() -> vk::CommandPool {
        Rvk_y("create cmd pool") = [&] {
            return mDevice.createCommandPool(vk::CommandPoolCreateInfo()
                            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                            .setQueueFamilyIndex(mQueueIndex));
        };
    }

    auto vkCommand::createCommandBuffers(uint32_t cmdCount) -> std::vector<vk::CommandBuffer> {
        Rvk_y("create cmd") = [&]{
            return mDevice.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo().setCommandPool(mCommandPool)
                                                            .setCommandBufferCount(cmdCount)
                                                            .setLevel(vk::CommandBufferLevel::ePrimary));
        };
    }

    auto vkCommand::beginCommandBuf(vk::Extent2D extent2D) -> void {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

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
    }

    auto vkCommand::endCommandBuf() -> void {
        mActiveCommandBuffer.end();
    }

    auto vkCommand::beginRenderPass(vkRenderPassInfo passInfo) -> void {
        auto imageIndex = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        std::vector<vk::ClearValue> cv{vk::ClearColorValue{0.f, 0.f, 0.f, 0.f}};

        vk::RenderPassBeginInfo renderPassBeginInfo{passInfo.renderPass, passInfo.framebuffers[imageIndex],
                                                    {passInfo.offset2D, passInfo.extent}, passInfo.clearValues};

        mActiveCommandBuffer.beginRenderPass(renderPassBeginInfo, passInfo.subpassContents);
    }

    auto vkCommand::endRenderPass() -> void {
        mActiveCommandBuffer.endRenderPass();
    }


} // yic