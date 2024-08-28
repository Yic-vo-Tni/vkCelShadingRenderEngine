//
// Created by lenovo on 7/22/2024.
//

#include "ModelManager.h"

namespace sc {



    ModelManager::ModelManager(const flecs::world* ecs) : ecs(ecs){
        p = {
            .clearColor = glm::vec4(1, 0, 0, 1.00f),
            .lightPosition =  {5.f, 15.f, 8.f},
            .lightIntensity = 100.f,
            .lightType = 0
        };

        mRenderGroupGraphics = yic::RenderGroupGraphics ::configure(yic::FrameRender::eColorDepthStencilRenderPass)
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
                ->addDesSetLayout_(0, 1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
                ->addBindingDescription_(0, sizeof(sc::Vertex), vk::VertexInputRate::eVertex)
                ->addAttributeDescription_(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(sc::Vertex, pos))
                ->addAttributeDescription_(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, nor))
                ->addAttributeDescription_(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv))
                ->addShader_("model_v.vert", vk::ShaderStageFlagBits::eVertex)
                ->addShader_("model_f.frag", vk::ShaderStageFlagBits::eFragment)
                ->build()
                ;

        mRenderGroupRayTracing = yic::RenderGroupRayTracing::configure()
                ->addDesSetLayout_(0, 0, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addDesSetLayout_(0, 1, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eRaygenKHR)
                ->addDesSetLayout_(0, 2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addDesSetLayout_(0, 3, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eClosestHitKHR)
                ->addPushConstantRange_(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR, 0, sizeof(PushConstantRay))
                ->addShader_("rt_gen.rgen", vk::ShaderStageFlagBits::eRaygenKHR)
                ->addShader_("rt_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("rt_shadow_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR)
                ->addShader_("rt_hit.rchit", vk::ShaderStageFlagBits::eClosestHitKHR)
                ->build()
                ;

        subscribeModel();
        globalCamera.initBuf();

        mRTBuilder = std::make_unique<yic::RTBuilder>();
        mRTBuilder->cDesSets(globalCamera.getVpMatrixBuf());
    }

    ModelManager::~ModelManager() {
        mRenderGroupGraphics.reset();
        mRenderGroupRayTracing.reset();
        globalCamera.clear();
    }

    auto ModelManager::Render(const vk::CommandBuffer &cmd) -> void {
        oneapi::tbb::spin_rw_mutex::scoped_lock lock(mModelMutex, false);

        auto query = ecs->query<Model::Generic>();

        yic::EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext &uiWidgetContext) {
            mExtent.store(vk::Extent2D{(uint32_t) uiWidgetContext.viewportSize.value().x,
                                       (uint32_t) uiWidgetContext.viewport_height_v()});
            mRtStorageOffImg = yic::Allocator::allocImgOffScreen_Storage(mExtent, 1);

            query.each([&](flecs::entity e, sc::Model::Generic &model) {
                auto& uniqueCmd = model.cmd;

                vk::CommandBufferInheritanceInfo inheritanceInfo{
                        yic::FrameRender::eColorDepthStencilRenderPass, 0
                };
                vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse, &inheritanceInfo};

                uniqueCmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
                uniqueCmd.begin(beginInfo);
                vk::Viewport viewport{
                        0.f, 0.f,
                        static_cast<float>(mExtent.load().width), static_cast<float>(mExtent.load().height),
                        0.f, 1.f
                };
                vk::Rect2D scissor{{0, 0}, mExtent};
                uniqueCmd.setViewport(0, viewport);
                uniqueCmd.setScissor(0, scissor);
                uniqueCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->acquire());
                for (const auto &mesh: model.meshes) {
                    uniqueCmd.bindVertexBuffers(0, mesh.vertBuf->buffer, {0});
                    uniqueCmd.bindIndexBuffer(mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                    uniqueCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                 mRenderGroupGraphics->getPipelineLayout(), 0,
                                                 model.descriptor->getDescriptorSets()[mesh.texIndex], nullptr);
                    uniqueCmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
                }
                uniqueCmd.end();

            });
        }, "ModelManager");

        mRenderGroupGraphics->render(cmd)
                ->bindModelSecondary(query);
    }

    auto ModelManager::renderRt(const vk::CommandBuffer &cmd) -> void {
        oneapi::tbb::spin_rw_mutex::scoped_lock lock(mModelMutex, false);

        mRTBuilder->draw(cmd);
//        auto query = ecs->query<Model::Generic>();
//        query.each([&](flecs::entity e, sc::Model::Generic &model) {
//            size_t desSetCount = model.rtDescriptor->getDescriptorSets().size();
////            for (int i = 0; i < model.meshes.size(); i++) {
////                cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR,
////                                       mRenderGroupRayTracing->getPipelineLayout(), 0,
////                                       i < desSetCount ? model.rtDescriptor->getDescriptorSets()[i] : model.rtDescriptor->getDescriptorSets().back(),
////                                       nullptr);
//                cmd.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR,
//                                       mRenderGroupRayTracing->getPipelineLayout(), 0,
//                                       model.rtDescriptor->getDescriptorSets().back(),
//                                       nullptr);
//                cmd.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, mRenderGroupRayTracing->acquire());
//                cmd.pushConstants(mRenderGroupRayTracing->getPipelineLayout(),
//                                  vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR |
//                                  vk::ShaderStageFlagBits::eMissKHR,
//                                  0, sizeof(PushConstantRay), &p);
////                cmd.pushConstants(mRenderGroupRayTracing->getPipelineLayout(),
////                                  vk::ShaderStageFlagBits::eRaygenKHR |
////                                  vk::ShaderStageFlagBits::eMissKHR,
////                                  0, sizeof(PushConstantRay), &p);
//                auto extent = mExtent.load();
//                cmd.traceRaysKHR(&mRenderGroupRayTracing->getRegionRgen(),
//                                 &mRenderGroupRayTracing->getRegionMiss(),
//                                 &mRenderGroupRayTracing->getRegionHit(),
//                                 &mRenderGroupRayTracing->getRegionCall(),
//                                 extent.width, extent.height, 1,
//                                 yic::EventBus::Get::vkSetupContext().dynamicDispatcher_ref());
////            }
//
//        });
    }


    auto ModelManager::subscribeModel() -> void {
        yic::EventBus::subscribeAuto([&](const et::eResPaths& paths){
            for(const auto& [res, pts] : paths.paths_v()){
                auto loadModel = [&](const ResFormat &format) {
                    if (res == format){
                        for(auto& pt : pts){
                            auto model = ModelLoader::Load(pt, *mRenderGroupGraphics);
                            model.shaderPaths = mRenderGroupGraphics->getShaderPaths();
                            model.descriptor = yic::Descriptor::configure(*mRenderGroupGraphics)
                                    ->updateDesSetAuto(globalCamera.getVpMatrixBuf(), model.diffTexs);
                            std::vector<vkBuf_sptr> addrBufs;
                            for(auto& mesh : model.meshes){
                                addrBufs.emplace_back(mesh.addrBuf);
                            }
                            model.rtDescriptor = yic::Descriptor::configure(*mRenderGroupRayTracing)
                                    ->updateDesSetAuto(model.tlas->accel,
                                                       yic::ImgInfo{nullptr, mRtStorageOffImg->imageViews, vk::ImageLayout::eGeneral},
                                                       globalCamera.getVpMatrixBuf(), addrBufs.front());
//                            model.rtDescriptor = yic::Descriptor::configure(*mRenderGroupRayTracing)
//                                    ->updateDesSetAuto(model.tlas->accel,
//                                                       yic::ImgInfo{nullptr, mRtStorageOffImg->imageViews, vk::ImageLayout::eGeneral},
//                                                       globalCamera.getVpMatrixBuf());

                            model.cmd = yic::CommandBufferCoordinator::cmdDrawSecond(yic::FrameRender::eColorDepthStencilRenderPass, mExtent.load(), [&](vk::CommandBuffer& cmd){
                                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->acquire());
                                for (const auto &mesh: model.meshes) {
                                    cmd.bindVertexBuffers(0, mesh.vertBuf->buffer, {0});
                                    cmd.bindIndexBuffer(mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                                           model.descriptor->getDescriptorSets()[mesh.texIndex], nullptr);
                                    cmd.drawIndexed(mesh.indices.size(), 1, 0, 0, 0);
                                }
                            });



                            oneapi::tbb::spin_rw_mutex::scoped_lock lock(mModelMutex, true);
                            ecs->defer_begin();
                            ecs->entity(model.id.c_str()).set<Model::Generic>(model).add<Selected>();
                            ecs->defer_end();
                            vkInfo("add model successfully");
                        }
                    }
                };

                loadModel(ResFormat::eObj);
                loadModel(ResFormat::ePmx);
                loadModel(ResFormat::eFbx);
                loadModel(ResFormat::eGltf);
            }
        });

    }


} // sc








































//        yic::EventBus::subscribeAuto([&](const et::eResPathsNfd &pt) {
//            nfdchar_t *outPt;
//
//            auto file = pt.file.value();
//            auto format = file.first;
//            if (format == ResFormat::eVmd) {
//                auto r = NFD_OpenDialog("vmd", nullptr, &outPt);
//                if (r == NFD_OKAY) {
//                    auto pmx = static_cast<Model::Pmx *>(file.second);
//                    ModelLoader::Vmd(outPt, *pmx);
//
//                    vkInfo("add vmd successfully for {0}", pmx->id);
//                }
//            }
//        });
//
//        ecs().system<Model::Pmx>().each([](flecs::entity e, Model::Pmx &pmx) {
//            if (pmx.vmd != nullptr) {
//                double time = saba::GetTime();
//                double elapsed = time - pmx.saveTime;
//                if (elapsed > 1.f / 30.f) {
//                    elapsed = 1.f / 30.f;
//                }
//                pmx.saveTime = time;
//                pmx.animTime += elapsed;
//
//                pmx.pmx->BeginAnimation();
//                pmx.pmx->UpdateAllAnimation(pmx.vmd.get(), (float)pmx.animTime * 30.f, (float)elapsed);
//                pmx.pmx->EndAnimation();
//
//                pmx.pmx->Update();
//                auto pos = pmx.pmx->GetUpdatePositions();
//                const auto &nor = pmx.pmx->GetUpdateNormals();
//                const auto &uvs = pmx.pmx->GetUpdateUVs();
//                std::vector<Vertex> vertices;
//                for (size_t i = 0; i < pmx.pmx->GetVertexCount(); i++) {
//                    auto originUv = uvs[i];
//                    auto flipUv = glm::vec2(originUv.x, 1.f - originUv.y);
//                    vertices.emplace_back(Vertex{pos[i], nor[i], flipUv});
//                }
//                pmx.vertBuf->updateBuf(vertices);
//            }
//        });





































