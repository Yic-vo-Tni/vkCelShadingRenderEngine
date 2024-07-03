//
// Created by lenovo on 6/10/2024.
//

#include "vkCommand.h"

namespace yic {

    vkCommand::vkCommand(const std::string &id, const uint32_t &qIndex)
            : mId(id),
              mQueueIndex(qIndex),
              mDevice(EventBus::Get::vkSetupContext().device_ref()),
              mRenderPass(EventBus::Get::vkRenderContext(id).renderPass_ref()),
              mFrameBuffers(EventBus::Get::vkRenderContext(id).framebuffers_ref()),
              mCommandPool(createCommandPool()),
              mCommandBuffers(createCommandBuffers()) {

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

    auto vkCommand::createCommandBuffers() -> std::vector<vk::CommandBuffer> {
        Rvk_y("create cmd") = [&]{
            return mDevice.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo().setCommandPool(mCommandPool)
                                                            .setCommandBufferCount(EventBus::Get::vkRenderContext(mId).imageCount_v())
                                                            .setLevel(vk::CommandBufferLevel::ePrimary));
        };
    }

    auto vkCommand::beginCommandBuf() -> void {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

        auto imageIndex = EventBus::Get::vkRenderContext(mId).activeImageIndex_v();
        auto framebuffers = EventBus::Get::vkRenderContext(mId).framebuffers_ref();
        auto extent = EventBus::Get::vkRenderContext(mId).currentExtent_v();
        mActiveCommandBuffer = mCommandBuffers[imageIndex];

        mActiveCommandBuffer.begin(beginInfo);
        {
            std::vector<vk::ClearValue> cv{vk::ClearColorValue{0.f, 0.f, 0.f, 0.f}};

            vk::RenderPassBeginInfo renderPassBeginInfo{mRenderPass, framebuffers[imageIndex],
                                                        {{0, 0}, extent}, cv};

            mActiveCommandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        }

        EventBus::update(et::vkRenderContext{.cmd = mActiveCommandBuffer}, mId);
    }

    auto vkCommand::endCommandBuf() -> void {
        mActiveCommandBuffer.endRenderPass();
        mActiveCommandBuffer.end();
    }




} // yic