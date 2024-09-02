//
// Created by lenovo on 7/22/2024.
//

#include "ModelManager.h"

namespace sc {

    ModelManager::ModelManager(const flecs::world* ecs) : ecs(ecs){
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

        subscribeModel();
        globalCamera.initBuf();

        mRTBuilder = std::make_unique<yic::RTBuilder>();
        mRTBuilder->cRTPipeAndSBT(globalCamera.getVpMatrixBuf());
    }

    ModelManager::~ModelManager() {
        mRenderGroupGraphics.reset();
        globalCamera.clear();
    }


    auto ModelManager::prepare() -> void {
        oneapi::tbb::spin_rw_mutex::scoped_lock lock(mModelMutex, false);

        rebuild();

        auto pri = yic::EventBus::val<et::pRenderProcess>(enum_name(RenderProcessPhases::ePrimary)).pProcess_ref();
        if (mRenderOffImg != nullptr && !mRenderOffImg->framebuffers.empty()){
            pri->appendRenderPassProcessSecondaryCommand(0, mRenderOffImg->framebuffers, [this](vk::CommandBuffer& cmd){
                auto query = ecs->query<Model>();
                mRenderGroupGraphics->render(cmd)
                        ->bindModelSecondary(query);
            });
        }
        pri->appendProcessCommand(1, [&](vk::CommandBuffer& cmd){
            mRTBuilder->drawNew(cmd);
        });
    }

    auto ModelManager::subscribeModel() -> void {
        yic::EventBus::subscribeAuto([&](const et::eResPaths& paths){
            for(const auto& [res, pts] : paths.paths_v()){
                auto loadModel = [&](const ResFormat &format) {
                    if (res == format){
                        for(auto& pt : pts){

                            auto model = ModelLoader::Load(pt);
                            model.info.shaderPts = mRenderGroupGraphics->getShaderPaths();
                            model.des.descriptor = yic::Descriptor::configure(*mRenderGroupGraphics)
                                    ->updateDesSetAuto(globalCamera.getVpMatrixBuf(), model.texs.diffTexs);

                            model.cmd = yic::CommandBufferCoordinator::cmdDrawSecond(yic::FrameRender::eColorDepthStencilRenderPass, mExtent.load(), [&](vk::CommandBuffer& cmd){
                                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->acquire());
                                for(const auto& subMesh : model.mesh.subMeshes){
                                    cmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
                                    cmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                                           model.des.descriptor->getDescriptorSets()[subMesh.texIndex], nullptr);
                                    cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                                }
                            });

                            oneapi::tbb::spin_rw_mutex::scoped_lock lock(mModelMutex, true);
                            ecs->defer_begin();
                            ecs->entity(model.info.id.c_str()).set<Model>(model).add<Selected>();
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

    auto ModelManager::rebuild() -> void {
        yic::EventBus::subscribeDeferredAuto([&](const et::uiWidgetContext &uiWidgetContext) {
            auto pri = yic::EventBus::val<et::pRenderProcess>(enum_name(RenderProcessPhases::ePrimary)).pProcess_ref();
            auto query = ecs->query<Model>();
            mExtent.store(vk::Extent2D{(uint32_t) uiWidgetContext.viewportSize.value().x,
                                       (uint32_t) uiWidgetContext.viewport_height_v()});
            mRtStorageOffImg = yic::Allocator::allocImgOffScreen_Storage(mExtent, 1);
            mRenderOffImg = yic::Allocator::allocImgOffScreen_DepthStencilAndFramebuffers(yic::vkImageConfig{mExtent}, yic::FrameRender::eColorDepthStencilRenderPass,
                                                                                          enum_name(RenderProcessPhases::ePrimary), 3);
            pri->updateDescriptor(static_cast<int>(RenderProcessPhases::ePrimary), mRenderOffImg);

            query.each([&](flecs::entity e, sc::Model &model) {
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
                for (const auto &subMesh: model.mesh.subMeshes) {
                    uniqueCmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
                    uniqueCmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                    uniqueCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                 mRenderGroupGraphics->getPipelineLayout(), 0,
                                                 model.des.descriptor->getDescriptorSets()[subMesh.texIndex], nullptr);
                    uniqueCmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                }
                uniqueCmd.end();
            });
        }, "ModelManager");
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





































