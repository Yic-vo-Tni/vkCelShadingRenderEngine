//
// Created by lenovo on 6/10/2024.
//

#include "vkFrameRender.h"

namespace yic {

    vkFrameRender::vkFrameRender() : mDevice(EventBus::Get::vkSetupContext().device_v()) {

    }

    vkFrameRender::~vkFrameRender() {

    }

    auto vkFrameRender::createRenderPass(const std::vector<vk::AttachmentDescription> &attachDes,
                                         const std::vector<vk::SubpassDescription> &subpass,
                                         const std::vector<vk::SubpassDependency> &dependency, const std::string& des) -> vk::RenderPass {
        vk::RenderPassCreateInfo createInfo{{}, attachDes, subpass, dependency};

        Rvk_t("create " + des + " render pass", spdlog::level::warn) = [&] {
            return mDevice.createRenderPass(createInfo);
        };
    }

    auto vkFrameRender::createFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent, const std::vector<std::vector<vk::ImageView>>& attach) -> std::vector<vk::Framebuffer> {
        std::vector<vk::Framebuffer> framebuffers;
        framebuffers.resize(attach.size());

        Rvk_y("create frame buffer") = [&]{
            for(size_t i = 0; i < attach.size(); i++){
              auto createInfo = vk::FramebufferCreateInfo().setRenderPass(renderPass)
                                                    .setAttachments(attach[i])
                                                    .setWidth(extent.width)
                                                    .setHeight(extent.height)
                                                    .setLayers(1);

              framebuffers[i] = mDevice.createFramebuffer(createInfo);
            }

            return framebuffers;
        };
    }





} // yic