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
            mCommand = std::make_unique<vkCommand>(id, qIndex);
            mImGui = std::make_unique<vkImGui>(id, queue, qIndex);

            TaskBus::registerTask(tt::RenderTarget_s::eMainWindow, [this] {
                FrameLoop();
            });
        }


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

/// alloc image / buf test
//        auto img = vot::concurrent_shared_ptr_unordered_map<vkImage>(vkAllocator::allocImg(img_path "/4.png"));
//        EventBus::update(et::vkResource{
//            .img = off_image
//        });
//        auto y = EventBus::Get::vkResource().img_ref().find_ref(img.tempId);
//        vkError(y->id);
//        vkError(EventBus::Get::vkResource().img_ref().size());
//
//        auto buf = vot::concurrent_shared_ptr_unordered_map<vkBuffer>(vkAllocator::allocBuf(5, vk::BufferUsageFlagBits::eTransferDst, vkAllocator::MemoryUsage::eCpuOnly, IdGenerator::uniqueId()));
//        EventBus::update(et::vkResource{
//            .buf = buf
//        });
//        vkError(EventBus::Get::vkResource().buf_ref().size());
//        auto t = EventBus::Get::vkResource().buf_ref().find_ref(buf.tempId)->id;
//        auto t_1 = EventBus::Get::vkResource().buf_ref().find_ref(t)->id;
//        auto buf_2 = vkAllocator::allocBuf(5, vk::BufferUsageFlagBits::eTransferDst, vkAllocator::MemoryUsage::eCpuOnly, IdGenerator::uniqueId());
//        EventBus::update(et::vkResource{
//            .buf = buf_2
//        });
//        vkError(EventBus::Get::vkResource().buf_ref().size());
//        auto x = EventBus::Get::vkResource().buf_ref();
//        EventBus::Get::vkResource().buf_ref().find_ref(buf_2->id)->updateBuf(5);
//        vkError(t);
//        vkError(t_1);

/// pipeline / set test
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

//        VkDeviceSize size;
//        std::vector<int> t(5);
//        auto buf = vkAllocator::allocBuf(sizeof (int) * t.size(), vk::BufferUsageFlagBits::eVertexBuffer, vkAllocator::MemoryUsage::eCpuToGpu);
//        t.emplace_back(1);
//        buf->updateBuf(t);

//        img = vkAllocator::allocImg(img_path "/4.png");
//
//        auto offImg = vkAllocator::allocImgOffScreen(vkImageConfig{1920, 1080});
//        mSampler = EventBus::Get::vkSetupContext().device->createSampler(samplerCreateInfo);
//
//        mDescriptor.addDesSetLayout({
//            vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment},
//        });
//
//        mDescriptor.createDesPool()
//                   // .pushbackDesSets(0)
//                    .updateDesSet({
//                        vk::DescriptorImageInfo{mSampler, img->imageView, vk::ImageLayout::eShaderReadOnlyOptimal},
//                    });

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
        mCommand->beginCommandBuf();

        auto rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);
        configureDynamicViewportScissor(rt);

        auto cmd = rt.cmd_ref();
        auto imageIndex = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).activeImageIndex_v();

        std::vector<vk::ClearValue> cv{vk::ClearColorValue{0.f, 0.f, 0.f, 0.f}};
        vk::RenderPassBeginInfo renderPassBeginInfo{renderPass, frameBuffers[imageIndex],
                                                    {{0, 0}, ext}, cv};

        cmd.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline->Get());
        cmd.draw(3, 1, 0, 0);

        cmd.endRenderPass();

        mCommand->beginRenderPass();

        mImGui->render();

        mCommand->endRenderPass();
        mCommand->endCommandBuf();
        mSwapchain->submitFrame();

        sf::Time frameTime = mClock.getElapsedTime() - start;
        if (frameTime < mTimePerFrame){
            sf::sleep(mTimePerFrame - frameTime);
        }

        return true;
    }

    auto vkRhi::configureDynamicViewportScissor(et::vkRenderContext rt) -> void {
        auto cmd = rt.cmd_ref();
        vk::Viewport viewport{0.f, 0.f,
                              static_cast<float>(rt.width_v()), static_cast<float>(rt.height_v()),
                              0.f, 1.f
        };
        cmd.setViewport(0, viewport);
        vk::Rect2D scissor{{0, 0}, rt.extent_v()};
        cmd.setScissor(0, scissor);
    }


} // yic