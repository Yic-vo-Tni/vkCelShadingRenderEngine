//
// Created by lenovo on 7/22/2024.
//

#include "ModelManager.h"
#include "Engine/ECS/Model/ModelLoader.h"

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

        mModelQuery = ecs->query<Model>();
        mRenderHandle = yic::EventBus::val<et::pRenderProcess>(enum_name(RenderPhase::ePrimary)).pProcess_ref();

        mRenderTargetOffImg = mg::Allocator->allocImage(yic::ImageConfig()
                .setExtent(mExtent)
                .setImageFlags(yic::ImageFlags::eDepthStencil)
                .setRenderPass(yic::FrameRender::eColorDepthStencilRenderPass)
                .setImageCount(3), " enum(PrimaryRenderSeq::Graphics)");

        mRenderHandle->updateDescriptor(static_cast<int>(RenderPhase::ePrimary), mRenderTargetOffImg);

        mRTBuilder = std::make_unique<yic::RTBuilder>();
        mRTBuilder->cRTPipeAndSBT(globalCamera.getVpMatrixBuf());

        mRenderHandle->appendRenderPassProcessSecondaryCommand(PrimaryRenderSeq::eGraphics, mRenderTargetOffImg, [this](vk::CommandBuffer& cmd){
            mModelQuery.each([&](flecs::entity e, sc::Model& m){
                cmd.executeCommands(m.cmd);
            });
        });
        mRenderHandle->appendProcessCommand(PrimaryRenderSeq::eRT, [&](vk::CommandBuffer& cmd){
            mRTBuilder->drawNew(cmd);
        });
    }

    ModelManager::~ModelManager() {
        mRenderGroupGraphics.reset();
    }


    auto ModelManager::prepare() -> void {
        oneapi::tbb::spin_rw_mutex::scoped_lock lock(mSubscribeModelMutex, false);

        rebuild();
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

                            model.cmd = yic::CommandBufferCoordinator::cmdDrawSecond(yic::FrameRender::eColorDepthStencilRenderPass, mExtent, [&](vk::CommandBuffer& cmd){
                                cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->acquire());
                                for(const auto& subMesh : model.mesh.subMeshes){
                                    cmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
                                    cmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                                    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mRenderGroupGraphics->getPipelineLayout(), 0,
                                                           model.des.descriptor->getDescriptorSets()[subMesh.texIndex], nullptr);
                                    cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                                }
                            });

                            oneapi::tbb::spin_rw_mutex::scoped_lock lock(mSubscribeModelMutex, true);
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
            mExtent = (vk::Extent2D{(uint32_t) uiWidgetContext.viewportSize.value().x, (uint32_t) uiWidgetContext.viewport_height_v()});

            mRenderTargetOffImg = mg::Allocator->allocImage(yic::ImageConfig()
                                                                    .setExtent(mExtent)
                                                                    .setImageFlags(yic::ImageFlags::eDepthStencil)
                                                                    .setRenderPass(yic::FrameRender::eColorDepthStencilRenderPass)
                                                                    .setImageCount(3), " enum(PrimaryRenderSeq::Graphics)");

            mRenderHandle->updateDescriptor(static_cast<int>(RenderPhase::ePrimary), mRenderTargetOffImg);

            mModelQuery.each([&](flecs::entity e, sc::Model &model) {
                auto& uniqueCmd = model.cmd;

                vk::CommandBufferInheritanceInfo inheritanceInfo{
                        yic::FrameRender::eColorDepthStencilRenderPass, 0
                };
                vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eRenderPassContinue | vk::CommandBufferUsageFlagBits::eSimultaneousUse, &inheritanceInfo};

                uniqueCmd.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
                uniqueCmd.begin(beginInfo);
                vk::Viewport viewport{
                        0.f, 0.f,
                        static_cast<float>(mExtent.width), static_cast<float>(mExtent.height),
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








































































