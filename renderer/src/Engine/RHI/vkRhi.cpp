//
// Created by lenovo on 5/23/2024.
//

#include "vkRhi.h"

namespace yic {

    vkRhi::vkRhi() {
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelF{};
        vkWindow::get();
        vkInit::get(std::make_shared<vkInitCreateInfo>()
                            ->addInstanceLayers("VK_LAYER_KHRONOS_validation")
                            ->addInstanceExtensions(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_SWAPCHAIN_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
                            ->addPhysicalExtensions(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, &accelF)

                            ->setQueuesPriority(std::vector<float>{1.f, 0.9f})
        );

        auto ct = EventBus::Get::vkSetupContext();
        {
            auto id = et::vkRenderContext::id::mainRender;
            auto queue = ct.qGraphicsPrimary_ref();
            auto qIndex = ct.qIndexGraphicsPrimary_v();

            mSwapchain = std::make_unique<vkSwapchain>(id, queue, qIndex);
            mImGui = std::make_unique<vkImGui>(id, queue, qIndex);

            TaskBus::registerTask(tt::RenderTarget_s::eMainWindow, [this] {
                FrameLoop();
            });
        }

        ///////////////////////

        t();
        ////////////////////////

        mTimePerFrame = sf::seconds(1.f / 120.f);
    }

    vkRhi::~vkRhi() {
        EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
        EventBus::Get::vkSetupContext().device_ref().waitIdle();

        EventBus::destroy<et::vkResource>();
    }

    bool vkRhi::FrameLoop() {
        sf::Time start = mClock.getElapsedTime();

        TaskBus::executeShaderTask();

        mSwapchain->updateEveryFrame();

        ///////////////////////////////////////////////////////////////////////

        t_cmd->beginCommandBuf(vk::Extent2D{1920, 1080});
        auto cmd = EventBus::Get::vkCommandBuffer("t").cmd_ref();
        auto imageIndex = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();
        t_cmd->beginRenderPass(vkCommand::vkRenderPassInfo{
           renderPass, frameBuffers, ext, vkCommand::vkClearValueBuilder::colorClearValue()
        });

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Get());
        cmd.draw(3, 1, 0, 0);

        t_cmd->endRenderPass();
        t_cmd->endCommandBuf();

        //////////////////////////////////////////////////////////////////////


        std::vector<vk::CommandBuffer> cmds = {
            cmd
        };
        mSwapchain->submitFrame([this]{
            mImGui->render();
        }, cmds);

        sf::Time frameTime = mClock.getElapsedTime() - start;
        if (frameTime < mTimePerFrame){
            sf::sleep(mTimePerFrame - frameTime);
        }

        return true;
    }

    auto vkRhi::t() -> void {
        auto ct = EventBus::Get::vkSetupContext();
        t_cmd = std::make_unique<vkCommand>("t", ct.qIndexGraphicsPrimary_v(), EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).imageCount_v());
        ext = vk::Extent2D{1920, 1080};
        off_image = vkAllocator::allocImgOffScreen(vkImageConfig{ext}, "m", EventBus::Get::vkRenderContext(
                et::vkRenderContext::id::mainRender).imageCount_v());
        EventBus::update(et::vkResource{
                .img = off_image
        });
        auto device = EventBus::Get::vkSetupContext().device_ref();
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

        renderPass = vkCreate("create render pass", spdlog::level::warn) = [&] {
            return device.createRenderPass(createInfo_r);
        };

        for(size_t i = 0; i < EventBus::Get::vkRenderContext(
                et::vkRenderContext::id::mainRender).imageCount_v(); i++) {
            auto view = off_image->imageViews[i];

            auto createInfo = vk::FramebufferCreateInfo().setRenderPass(renderPass)
                    .setAttachments(view)
                    .setWidth(ext.width)
                    .setHeight(ext.height)
                    .setLayers(1);

            frameBuffers.emplace_back(device.createFramebuffer(createInfo));
        }

        descriptor = std::make_shared<vkDescriptor>("t");
        descriptor->addDesSetLayout({
                                            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
                                    });
        descriptor->createDesPool(3);
        for (auto i = 0; i < 3; i++) {
            descriptor->updateDesSet(
                    {
                            vk::DescriptorImageInfo{off_image->sampler, off_image->imageViews[i],
                                                    vk::ImageLayout::eShaderReadOnlyOptimal},
                    }
            );
        }
        EventBus::update(et::vkResource{
                .desc = descriptor
        });

        pipeline = std::make_unique<vkPipeline<Graphics>>(EventBus::Get::vkSetupContext().device_ref(), descriptor->getPipelineLayout(),
                                                          renderPass);
        pipeline->addShader(spv_path "/v_test.spv", vk::ShaderStageFlagBits::eVertex)
                .addShader(spv_path "/f_test.spv", vk::ShaderStageFlagBits::eFragment)
                .create();

    }


} // yic