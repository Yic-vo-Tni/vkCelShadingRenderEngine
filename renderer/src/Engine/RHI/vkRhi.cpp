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

//        auto info = vk::PipelineLayoutCreateInfo();
//        layout = EventBus::Get::vkSetupContext().device_ref().createPipelineLayout(info);
//        pipeline = std::make_unique<vkPipeline<Graphics>>(EventBus::Get::vkSetupContext().device_ref(), layout,
//                                      EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender).renderPass_ref());
//        pipeline->addShader(spv_path "/v_test.spv", vk::ShaderStageFlagBits::eVertex)
//                .addShader(spv_path "/f_test.spv", vk::ShaderStageFlagBits::eFragment)
//                .create();

//        VkDeviceSize size;
//        std::vector<int> t(5);
//        auto buf = vkAllocator::allocBuf(sizeof (int) * t.size(), vk::BufferUsageFlagBits::eVertexBuffer, vkAllocator::MemoryUsage::eCpuToGpu);
//        t.emplace_back(1);
//        buf->updateBuf(t);

//        img = vkAllocator::allocImg(img_path "/4.png");
//
//        auto offImg = vkAllocator::allocImgOffScreen(vkImageConfig{1920, 1080});
//
//        vk::SamplerCreateInfo samplerCreateInfo{{}, vk::Filter::eLinear, vk::Filter::eNearest, vk::SamplerMipmapMode::eLinear,
//                                                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
//                                                0.f, vk::False, 1.f,
//                                                vk::False, vk::CompareOp::eAlways,
//                                                0.f, 0.f,
//                                                vk::BorderColor::eIntOpaqueBlack, vk::False};
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
        EventBus::Get::vkSetupContext().device_ref().destroy(layout);

        EventBus::Get::vkSetupContext().qGraphicsPrimary_ref().waitIdle();
        EventBus::Get::vkSetupContext().device_ref().waitIdle();
    }

    bool vkRhi::FrameLoop() {
        sf::Time start = mClock.getElapsedTime();

        TaskBus::executeShaderTask();

        mSwapchain->updateEveryFrame();
        mCommand->beginCommandBuf();


        auto rt = EventBus::Get::vkRenderContext(et::vkRenderContext::id::mainRender);

        configureDynamicViewportScissor(rt);

        mImGui->render();

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