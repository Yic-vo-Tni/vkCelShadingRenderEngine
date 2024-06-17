//
// Created by lenovo on 6/10/2024.
//

#include "vkCommand.h"

namespace yic {

    vkCommand::vkCommand() : mDevice(EventBus::Get::vkDeviceContext().device.value()),
                             mRenderPass(EventBus::Get::vkRenderContext().renderPass_v()),
                             mFrameBuffers(EventBus::Get::vkRenderContext().framebuffers_v()),
                             mCommandPool(createCommandPool()),
                             mCommandBuffers(createCommandBuffers()) {

    }

    vkCommand::~vkCommand() {
        mDevice.destroy(mCommandPool);
    }

    auto vkCommand::createCommandPool() -> vk::CommandPool {
        Rvk_y("create cmd pool") = [&]{
            return mDevice.createCommandPool(
                    vk::CommandPoolCreateInfo().setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                            .setQueueFamilyIndex(EventBus::Get::vkDeviceContext().queueFamily->getImGuiGraphicsFamilyIndex()));
        };
    }

    auto vkCommand::createCommandBuffers() -> std::vector<vk::CommandBuffer> {
        Rvk_y("create cmd") = [&]{
            return mDevice.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo().setCommandPool(mCommandPool)
                                                            .setCommandBufferCount(EventBus::Get::vkRenderContext().imageCount_v())
                                                            .setLevel(vk::CommandBufferLevel::ePrimary));
        };
    }

    auto vkCommand::beginCommandBuf() -> vk::CommandBuffer {
        vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};

        auto imageIndex = EventBus::Get::vkRenderContext().activeImageIndex_v();
        auto& cmd = mCommandBuffers[imageIndex];
        //auto framebuffers = EventBus::Get::vkFrameRenderContext(et::vkFrameRenderContext::id::imguiRender).framebuffers.value();
        auto framebuffers = EventBus::Get::vkRenderContext().framebuffers_v();

        cmd.begin(beginInfo);
        {
            std::vector<vk::ClearValue> cv{vk::ClearColorValue{1.f, 0.f, 0.f, 0.f}};

            vk::RenderPassBeginInfo renderPassBeginInfo{mRenderPass, framebuffers[imageIndex],
                                                        {{0, 0}, EventBus::Get::vkWindowContext().extent.value()}, cv};

            cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
        }

        return cmd;
    }

    auto vkCommand::endCommandBuf(vk::CommandBuffer& cmd) -> void {
        cmd.endRenderPass();
        cmd.end();
    }




} // yic