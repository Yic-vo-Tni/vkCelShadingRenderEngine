//
// Created by lenovo on 5/20/2025.
//

#include "Core/DispatchSystem/SystemHub.h"
#include "RenderSubmissionSystem.h"
#include "Editor/ImGuiHub.h"
#include "RHI/Command.h"
#include "ECS/Camera/Camera.h"
#include "RenderLibrary.h"
#include "SM/Scene.h"
#include "SM/illuminate/DirectionLight.h"
#include "RS/ResourceSystem.h"

namespace sc {

    RenderSubmissionSystem::RenderSubmissionSystem(entt::registry& registry) : ecs{registry}{
        ct = yic::systemHub.val<ev::pVkSetupContext>();
        rt = yic::systemHub.val<ev::pVkRenderContext>();

        RHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);
    }

    RenderSubmissionSystem::~RenderSubmissionSystem() = default;

    auto RenderSubmissionSystem::flow(vot::CommandBuffer &cmd) -> void {
        yic::renderLibrary->RT_Main->drawRendering(cmd, [&]{ drawing_gBuffer(cmd); });
        yic::renderLibrary->RT_ShadowMap->drawRendering(cmd, [&]{ drawing_shadowMap(cmd); });
        yic::renderLibrary->RT_Volumetric_Clouds->drawRendering(cmd, [&]{ drawing_volumetric_clouds(cmd); });
        yic::renderLibrary->RT_Volumetric_Fog->drawRendering(cmd, [&]{ drawing_volumetric_fog(cmd); });
        yic::renderLibrary->RT_RayTracing->drawRender(cmd, draw_RTShadowCI(), [&]{ draw_RTShadow(cmd); });
        yic::renderLibrary->RT_Post->drawRendering(cmd, [&]{ drawing_post(cmd); });
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

    auto RenderSubmissionSystem::drawing_gBuffer(vot::CommandBuffer &cmd) -> void {
        auto draw_meshes = [&](rhi::GraphicsPipeline& pipeline, auto view, auto bindIndexBuffer) {
            cmd.bindPipeline_(pipeline)
                    .bindDescriptorSets_(pipeline, ecs.get<sc::Camera>(GLOBAL::camera).DS);

            view.each([&](entt::entity e, const vot::RenderComponent& rc) {
                auto combMat = rc.baseMat * rc.zmoMat;
                cmd.bindVertexBuffers(rc.vertexBuffer);
                bindIndexBuffer(rc);
                cmd.pushConstants(pipeline.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &combMat);

                for(const auto& [index, subMeshes] : rc.subMeshes){
                    cmd.bindDescriptorSets_(pipeline, rc.dsHandle, index);
                    for(const auto& subMesh : subMeshes){
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
            });
        };


        cmd.setRenderArea_(vot::Resolutions::eQHDExtent);

        draw_meshes(yic::renderLibrary->GP_Basic,
                    ecs.view<const vot::RenderVisibleTag, const vot::RenderComponent>(entt::exclude<vot::MMDTag>),
                    [&](const vot::RenderComponent& rc) { cmd.bindIndexBuffer_(rc.indexBuffer); });
        draw_meshes(yic::renderLibrary->GP_Basic_PMX,
                    ecs.view<const vot::RenderVisibleTag, const vot::MMDTag, const vot::RenderComponent>(),
                    [&](const vot::RenderComponent& rc) { cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType); });
    }

    auto RenderSubmissionSystem::draw_RTShadowCI() -> vot::ImageDrawCI {
        return vot::ImageDrawCI()
                .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setNewLayout(vk::ImageLayout::eGeneral)
                .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
                .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                .setDstStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
                .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    }

    auto RenderSubmissionSystem::draw_RTShadow(vot::CommandBuffer &cmd) -> void {
        if (yic::sceneSystem->acquireActiveScene()->tlas != nullptr) {
            cmd.bindPipeline_(yic::renderLibrary->RP_Shadow)
            .bindDescriptorSets_(yic::renderLibrary->RP_Shadow, ecs.get<sc::Camera>(GLOBAL::camera).DS)
            .bindDescriptorSets_(yic::renderLibrary->RP_Shadow)
            .traceRaysKHR_(yic::renderLibrary->RP_Shadow, vot::Resolutions::eQHDExtent, 1,ct.dynamicDispatcher);
        }
    }

    auto RenderSubmissionSystem::drawing_volumetric_clouds(vot::CommandBuffer &cmd) -> void {
        cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
        .bindPipeline_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds)
        .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds, ecs.get<sc::Camera>(GLOBAL::camera).DS)
        .pushConstants(yic::renderLibrary->GP_Volumetric_Overcast_Clouds.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (float), &iTime);
        cmd.draw(3, 1, 0, 0);
    }

    auto RenderSubmissionSystem::drawing_volumetric_fog(vot::CommandBuffer &cmd) -> void {
        auto cam = ecs.get<sc::Camera>(GLOBAL::camera);
        auto lightMat = sm::DirectionLightTool::updateLightSpaceMat(glm::vec3(7.f, 3.f, 2.f), cam.getProj(), cam.getView());
        cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
        .bindPipeline_(yic::renderLibrary->GP_Volumetric_Fog)
        .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Fog, ecs.get<sc::Camera>(GLOBAL::camera).DS)
        .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Fog, *rt.activeImageIndex)
        .pushConstants(yic::renderLibrary->GP_Volumetric_Fog.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (glm::mat4), &lightMat);
        cmd.draw(3, 1, 0, 0);
    }

    auto RenderSubmissionSystem::drawing_post(vot::CommandBuffer &cmd) -> void {
        cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
                .bindDescriptorSets_(yic::renderLibrary->GP_Post, ecs.get<sc::Camera>(GLOBAL::camera).DS)
                .bindPipeline_(yic::renderLibrary->GP_Post)
                .bindDescriptorSets_(yic::renderLibrary->GP_Post, *rt.activeImageIndex)
                .draw(3, 1, 0, 0);
    }

    auto RenderSubmissionSystem::drawing_shadowMap(vot::CommandBuffer &cmd) -> void {
        auto draw_meshes = [&](rhi::GraphicsPipeline& pipeline, auto view, auto bindIndexBuffer) {
            auto cam = ecs.get<sc::Camera>(GLOBAL::camera);
            cmd.bindPipeline_(pipeline);

            view.each([&](entt::entity e, const vot::RenderComponent& rc) {
                auto lightMat = sm::DirectionLightTool::updateLightSpaceMat(glm::vec3(7.f, 3.f, 2.f), cam.getProj(), cam.getView() * rc.baseMat * rc.zmoMat);
                cmd.bindVertexBuffers(rc.vertexBuffer);
                bindIndexBuffer(rc);
                cmd.pushConstants(pipeline.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &lightMat);

                for(const auto& [index, subMeshes] : rc.subMeshes){
                    for(const auto& subMesh : subMeshes){
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
            });
        };


        cmd.setRenderArea_(vot::Resolutions::eQHDExtent);

        draw_meshes(yic::renderLibrary->GP_ShadowMap_Basic,
                    ecs.view<const vot::RenderVisibleTag, const vot::RenderComponent>(entt::exclude<vot::MMDTag>),
                    [&](const vot::RenderComponent& rc) { cmd.bindIndexBuffer_(rc.indexBuffer); });
        draw_meshes(yic::renderLibrary->GP_ShadowMap_PMX,
                    ecs.view<const vot::RenderVisibleTag, const vot::MMDTag, const vot::RenderComponent>(),
                    [&](const vot::RenderComponent& rc) { cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType); });
    }


} // sc