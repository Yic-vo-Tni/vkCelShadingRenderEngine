//
// Created by lenovo on 7/14/2024.
//

#include "vkRender.h"

namespace yic {

    vkRender::vkRender() {
        ct = EventBus::Get::vkSetupContext();
        rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);

        mCommand_Primary = std::make_unique<vkCommand>(mId_Primary, ct.qIndexGraphicsPrimary_v(), rt.imageCount_v());
        mOffImage_Primary = vkAllocator::allocImgOffScreen(vkImageConfig{{mExtent}}, mId_Primary, rt.imageCount_v());
        EventBus::update(et::vkResource{ .img = mOffImage_Primary});

        mRenderPass = createRenderPass();
        mFramebuffers = createFramebuffers(mOffImage_Primary, mRenderPass);

        mDescriptor_Primary = vkAllocator::allocDesc(mId_Primary);

        mDescriptor_Primary->addDesSetLayout({
            vk::DescriptorSetLayoutBinding{0,vk::DescriptorType::eCombinedImageSampler,
                                           1,vk::ShaderStageFlagBits::eFragment},
        });
        mDescriptor_Primary->updateDesSet(rt.imageCount_v(), {
                vkDescriptor::ImgInfo{mOffImage_Primary->imageViews},
        });

        mPipeline_Primary = std::make_unique<GraphicsPipeline>(ct.device_ref(),
                                                                   mDescriptor_Primary->getPipelineLayout(),
                                                                   mRenderPass);
        mPipeline_Primary->addShader("v_test.spv", vk::ShaderStageFlagBits::eVertex)
                .addShader("f_test.spv", vk::ShaderStageFlagBits::eFragment)
                .create();
    }

    vkRender::~vkRender() {
        ct.device_ref().destroy(mRenderPass);
        for(auto& fb : mFramebuffers){
            ct.device_ref().destroy(fb);
        }
    }


    auto vkRender::render() -> vk::CommandBuffer{
        mCommand_Primary->beginCommandBuf(mExtent);
        auto cmd = EventBus::Get::vkCommandBuffer(mId_Primary).cmd_ref();
        mCommand_Primary->beginRenderPass(vkCommand::RenderPassInfo{
                mRenderPass, mFramebuffers,
        });

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mPipeline_Primary->Get());
        cmd.draw(3, 1, 0, 0);

        mCommand_Primary->endRenderPass();
        mCommand_Primary->endCommandBuf();

        return cmd;
    }

    auto vkRender::createRenderPass() -> vk::RenderPass {
        std::vector<vk::AttachmentReference> attachRef{{0, vk::ImageLayout::eColorAttachmentOptimal}};
        std::vector<vk::AttachmentDescription> attachDes{
                {{},
                 vk::Format::eR8G8B8A8Unorm, vk::SampleCountFlagBits::e1,
                 vk::AttachmentLoadOp::eClear,
                 vk::AttachmentStoreOp::eStore,
                 vk::AttachmentLoadOp::eDontCare,
                 vk::AttachmentStoreOp::eDontCare,
                 vk::ImageLayout::eUndefined,
                 vk::ImageLayout::eShaderReadOnlyOptimal}
        };
        std::vector<vk::SubpassDescription> subpass{
                {
                        {}, vk::PipelineBindPoint::eGraphics, {},
                        attachRef[0], {}}
        };

        vk::RenderPassCreateInfo createInfo_r{{}, attachDes, subpass, {}};

        return vkCreate("create render pass", spdlog::level::warn) = [&] {
            return ct.device_ref().createRenderPass(createInfo_r);
        };
    }

    auto vkRender::createFramebuffers(const vkImg_sptr& imgSptr, vk::RenderPass renderPass) -> std::vector<vk::Framebuffer> {
        std::vector<vk::Framebuffer> framebuffers;
        for (size_t i = 0; i < imgSptr->info_.imageCount; i++) {
            auto view = imgSptr->imageViews[i];

            auto createInfo = vk::FramebufferCreateInfo().setRenderPass(renderPass)
                    .setAttachments(view)
                    .setWidth(imgSptr->info_.width)
                    .setHeight(imgSptr->info_.height)
                    .setLayers(1);

            framebuffers.emplace_back(ct.device_ref().createFramebuffer(createInfo));
        }
        return framebuffers;
    }

} // yic