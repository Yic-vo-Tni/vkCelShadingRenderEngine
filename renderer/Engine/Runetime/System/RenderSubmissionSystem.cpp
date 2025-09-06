//
// Created by lenovo on 5/20/2025.
//

#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
#include "RenderSubmissionSystem.h"
#include "Editor/ImGuiHub.h"
#include "RHI/Command.h"
#include "Runetime/Camera/Camera.h"
#include "RenderLibrary.h"
#include "SM/Scene.h"
#include "SM/illuminate/DirectionLight.h"
#include "RS/ResourceSystem.h"

namespace sc {

    RenderSubmissionSystem::RenderSubmissionSystem(entt::registry& registry) : ecs{registry}{
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();

        RHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);

        uRenderGraph = std::make_unique<RenderGraph>();
    }

    RenderSubmissionSystem::~RenderSubmissionSystem() = default;

    auto RenderSubmissionSystem::flow(vot::CommandBuffer &cmd) -> void {
        const auto fast = yic::indexRing.get(vot::LogicBufferType::eFast).render_cur();
        const auto slow = yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur();
        const auto set0 = ecs.get<vot::DescriptorSet0>(GLOBAL::set0).handles[fast];

        auto draw_meshes = [&](rhi::GraphicsPipeline& pipeline, auto view) {
            cmd.bindPipeline_(pipeline)
            .bindDescriptorSets_(pipeline, set0);

            view.each([&](entt::entity e, const vot::RenderComponent& rc) {
                const auto combMat = rc.baseMat * rc.zmoMat;
                cmd.bindVertexBuffers(rc.vertexBuffer[slow]);
                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                cmd.pushConstants(pipeline.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &combMat);

                for(const auto& [index, subMeshes] : rc.subMeshes){
                    cmd.bindDescriptorSets_(pipeline, rc.dsHandle, index);
                    for(const auto& subMesh : subMeshes){
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
            });
        };

        auto draw_id_buffer = [&](rhi::GraphicsPipeline& pipeline, auto view) {
            cmd.bindPipeline_(pipeline)
            .bindDescriptorSets_(pipeline, set0);

            view.each([&](entt::entity e, const vot::RenderComponent& rc) {
                const auto combMat = rc.baseMat * rc.zmoMat;
                const auto pushConstants = IDBufferPushConstant{combMat, static_cast<std::uint32_t>(e)};

                cmd.bindVertexBuffers(rc.vertexBuffer[slow]);
                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                cmd.pushConstants(pipeline.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0,
                                  sizeof(IDBufferPushConstant), &pushConstants);

                for (const auto& subMesh : rc.subMeshes | std::views::values | std::views::join) {
                    cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                }
            });
        };

        auto draw_meshes_shadowMap = [&](rhi::GraphicsPipeline& pipeline, auto view) {
            auto cam = ecs.get<sc::Camera>(GLOBAL::camera);
            cmd.bindPipeline_(pipeline);

            view.each([&](entt::entity e, const vot::RenderComponent& rc) {
                //auto lightMat = sm::DirectionLightTool::updateLightSpaceMat(glm::vec3(7.f, 3.f, 2.f), cam.getProj(), cam.getView() * rc.baseMat * rc.zmoMat);
                auto lightMat = glm::mat4(1.f); // bug
                cmd.bindVertexBuffers(rc.vertexBuffer[slow]);
                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                cmd.pushConstants(pipeline.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &lightMat);

                for (const auto& subMesh : rc.subMeshes | std::views::values | std::views::join) {
                    cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                }
            });
        };

        auto draw_volumetric_clouds = [&]{
            if (GLOBAL::showVolumetricClouds) {
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
                .bindPipeline_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds)
                .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds, set0)
                .pushConstants(yic::renderLibrary->GP_Volumetric_Overcast_Clouds.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (float), &iTime);
                cmd.draw(3, 1, 0, 0);
            } else {
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
                .bindPipeline_(yic::renderLibrary->GP_Dummy);
                cmd.draw(3, 1, 0, 0);
            }
        };

        auto draw_volumetric_fog = [&]{
            if (!GLOBAL::showVolumetricFog) {
                const auto cam = ecs.get<sc::Camera>(GLOBAL::camera);
                const auto lightMat = sm::DirectionLightTool::updateLightSpaceMat(glm::vec3(7.f, 3.f, 2.f), cam.getProj(), cam.getView());
                constexpr vk::FragmentShadingRateCombinerOpKHR vrsCombiner[] = {
                        vk::FragmentShadingRateCombinerOpKHR::eReplace,
                        vk::FragmentShadingRateCombinerOpKHR::eReplace,
                };
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
                cmd.setFragmentShadingRateKHR(vk::Extent2D{4, 4}, vrsCombiner, *ct.dynamicDispatcher);
                cmd.bindPipeline_(yic::renderLibrary->GP_Volumetric_Fog)
                .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Fog, set0)
                .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Fog, *rt.activeImageIndex)
                .pushConstants(yic::renderLibrary->GP_Volumetric_Fog.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (glm::mat4), &lightMat);
                cmd.draw(3, 1, 0, 0);
            } else {
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
                .bindPipeline_(yic::renderLibrary->GP_Dummy)
                .draw(3, 1, 0, 0);
            }
        };

        auto drawci_RTShadow = vot::ImageDrawCI()
                .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setNewLayout(vk::ImageLayout::eGeneral)
                .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                .setDstStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
        auto draw_RTShadow = [&]{
            if (yic::sceneSystem->acquireActiveScene()->tlas != nullptr) {
                cmd.bindPipeline_(yic::renderLibrary->RP_Shadow)
                .bindDescriptorSets_(yic::renderLibrary->RP_Shadow, set0)
                .bindDescriptorSets_(yic::renderLibrary->RP_Shadow)
                .traceRaysKHR_(yic::renderLibrary->RP_Shadow, vot::Resolutions::eQHDExtent, 1, ct.dynamicDispatcher);
            }
        };

        auto draw_post = [&]{
            cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
            .bindDescriptorSets_(yic::renderLibrary->GP_Post, set0)
            .bindPipeline_(yic::renderLibrary->GP_Post)
            .bindDescriptorSets_(yic::renderLibrary->GP_Post, *rt.activeImageIndex)
            .draw(3, 1, 0, 0);
        };


        /// ------------------------------------------------------------------///

        uRenderGraph->begin();

        ecs.view<const vot::mark::eVisible, const vot::RenderComponent, const vot::VertexDataComponent, const vot::AnimationComponent>(entt::exclude<vot::mark::eMMD>)
        .each([&](entt::entity e, const vot::RenderComponent &rc, const vot::VertexDataComponent& vdc, const vot::AnimationComponent& ac) {
            const uint32_t max_id = vdc.vertices_pmr[vot::VertexDataComponent::eAnim].size();
            constexpr auto localSize = 64u;
            const auto groupCount = (vdc.vertices_pmr[0].size() + localSize - 1) / localSize;

            ac.boneMatBuffer->update(ac.boneMats[slow]);
            cmd.bindPipeline_(yic::renderLibrary->CP_Skinning);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, yic::renderLibrary->CP_Skinning.acquirePipelineLayout(), 1, rc.dsHandle.va(), {});
            cmd.pushConstants(yic::renderLibrary->CP_Skinning.acquirePipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0, sizeof (std::uint32_t), &max_id);
            cmd.dispatch(groupCount, 1, 1);
        });


        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_GBuffer,
            .execute = [&]{
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent);

                cmd.bindPipeline_(yic::renderLibrary->GP_Basic_Assimp)
                            .bindDescriptorSets_(yic::renderLibrary->GP_Basic_Assimp, set0);

                            ecs.view<const vot::mark::eVisible, const vot::RenderComponent>(entt::exclude<vot::mark::eMMD>).each([&](entt::entity e, const vot::RenderComponent& rc) {
                                const auto combMat = rc.baseMat * rc.zmoMat;
                                cmd.bindVertexBuffers(rc.vertexBuffer[vot::VertexDataComponent::eAnim]);
                                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                                cmd.pushConstants(yic::renderLibrary->GP_Basic_Assimp.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &combMat);

                                for(const auto& [index, subMeshes] : rc.subMeshes){
                                    cmd.bindDescriptorSets_(yic::renderLibrary->GP_Basic_Assimp, rc.dsHandle, index);
                                    for(const auto& subMesh : subMeshes){
                                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                                    }
                                }
                            });

                //draw_meshes(yic::renderLibrary->GP_Basic_Assimp, ecs.view<const vot::mark::eVisible, const vot::RenderComponent>(entt::exclude<vot::mark::eMMD>));
                draw_meshes(yic::renderLibrary->GP_Basic_PMX, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());
            }
        });
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_ShadowMap,
            .execute = [&]{
                cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
                draw_meshes_shadowMap(yic::renderLibrary->GP_ShadowMap_Assimp, ecs.view<const vot::mark::eVisible, const vot::RenderComponent>(entt::exclude<vot::mark::eMMD>));
                draw_meshes_shadowMap(yic::renderLibrary->GP_ShadowMap_PMX, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());
            }
        });
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_IDBuffer,
            .execute = [&] {
                draw_id_buffer(yic::renderLibrary->GP_IDBuffer_Assimp, ecs.view<const vot::mark::eVisible, const vot::RenderComponent>(entt::exclude<vot::mark::eMMD>));
                draw_id_buffer(yic::renderLibrary->GP_IDBuffer, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());
            },
        });
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_Volumetric_Clouds,
            .execute = draw_volumetric_clouds,
        });
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_Volumetric_Fog,
            .inputs = { yic::renderLibrary->RT_GBuffer },
            .execute = draw_volumetric_fog,
        }); // un use
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RTX_RayTracing,
            .drawci = drawci_RTShadow,
            .execute = draw_RTShadow,
        });
        uRenderGraph->addPass({
            .target = yic::renderLibrary->RT_Post,
            .inputs = { yic::renderLibrary->RT_GBuffer },
            .execute = draw_post,
        });

        uRenderGraph->end(cmd);
    }

    auto RenderSubmissionSystem::frame() -> void {
        iTime += (1.f / 60.f);
        iTime += 1.f / std::max(GLOBAL::fps, 0.01f);;

        yic::command->bind(vot::SubmitInfo()
                                   .setRHandle(RHandle)
                                   .setQueueType(vot::queueType::eUndefined)
                                   .setWaitValues(vot::timelineStage::ePrepare)
                                   .setSignalValues(vot::timelineStage::eFinish)
                                   .setWaitStageMasks(vk::PipelineStageFlagBits::eTopOfPipe), [&](vot::CommandBuffer& cmd){
            flow(cmd);
        });
    }


} // sc