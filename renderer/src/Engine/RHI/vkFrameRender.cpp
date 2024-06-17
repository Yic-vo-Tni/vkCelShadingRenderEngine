//
// Created by lenovo on 6/10/2024.
//

#include "vkFrameRender.h"

namespace yic {

    vkFrameRender::vkFrameRender() : mDevice(EventBus::Get::vkSetupContext().device_v()) {
        createImguiFrameRenderPass();
    }

    vkFrameRender::~vkFrameRender() {
        mDevice.destroy(EventBus::Get::vkRenderContext().renderPass_v());
        auto frameBuffers = EventBus::Get::vkRenderContext().framebuffers_v();
        for(auto& fb : frameBuffers){
            mDevice.destroy(fb);
        }
    }

    auto vkFrameRender::createImguiFrameRenderPass() -> void {
        std::vector<vk::AttachmentReference> attachRef{{0, vk::ImageLayout::eColorAttachmentOptimal}};
        auto rp = createRenderPass(
                {{{},
                  EventBus::Get::vkRenderContext().surfaceFormat_v(), vk::SampleCountFlagBits::e1,
                  vk::AttachmentLoadOp::eClear,
                  vk::AttachmentStoreOp::eStore,
                  vk::AttachmentLoadOp::eDontCare,
                  vk::AttachmentStoreOp::eDontCare,
                  vk::ImageLayout::eUndefined,
                  vk::ImageLayout::ePresentSrcKHR},},
                {{
                         {}, vk::PipelineBindPoint::eGraphics, {},
                         attachRef[0], {}}},
                {}
        );

        auto createFrameBuf = [&, rp]{
            std::vector<std::vector<vk::ImageView>> attaches{};
            auto frameEntries = EventBus::Get::vkRenderContext().frameEntries_v();
            for (const auto& view: frameEntries) {
                std::vector<vk::ImageView> attach{view.imageView};
                attaches.push_back(attach);
            }
            auto fb = createFrameBuffer(rp, EventBus::Get::vkRenderContext().extent_v(), attaches);

            EventBus::update(et::vkRenderContext{.renderPass = rp, .framebuffers = fb});
        };
        createFrameBuf();

        TaskBus::registerTask(tt::RebuildSwapchain::eFrameBuffersRebuild, [=, this]{
            //auto fb = EventBus::Get::vkFrameRenderContext(et::vkFrameRenderContext::id::imguiRender).framebuffers.value();
            auto fb = EventBus::Get::vkRenderContext().framebuffers_v();
            for(const auto& f : fb){
                mDevice.destroy(f);
            }
            createFrameBuf();
        });

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