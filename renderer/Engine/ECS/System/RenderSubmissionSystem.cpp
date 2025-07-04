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
#include "RS/ResourceSystem.h"

namespace sc {

    RenderSubmissionSystem::RenderSubmissionSystem(entt::registry& registry) : ecs{registry}{
    //RenderSubmissionSystem::RenderSubmissionSystem(flecs::world &ecs) : ecs{ecs}{
        ct = yic::systemHub.val<ev::pVkSetupContext>();
        rt = yic::systemHub.val<ev::pVkRenderContext>();

        RHandle = yic::command->acquire(vot::threadSpecificCmdPool::eMainRender);
    }

    RenderSubmissionSystem::~RenderSubmissionSystem() = default;

    auto RenderSubmissionSystem::flow(vot::CommandBuffer &cmd) -> void {
        // 1
        yic::renderLibrary-> RT_Main->drawRendering(cmd, [&] {
            cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
            .bindPipeline_(yic::renderLibrary->GP_Basic)
            //.bindDescriptorSets_(yic::renderLibrary->GP_Basic, camera_comp(ecs)->DS);
            .bindDescriptorSets_(yic::renderLibrary->GP_Basic, ecs.get<sc::Camera>(GLOBAL::camera).DS);

            //ecs.query<const vot::RenderVisibleTag, const vot::RenderComponent>().each([&](flecs::entity_view e, const vot::RenderVisibleTag, const vot::RenderComponent& rc){
            ecs.view<const vot::RenderVisibleTag, const vot::RenderComponent>().each([&](entt::entity e, const vot::RenderComponent& rc){
                if (!ecs.all_of<vot::MMDTag>(e)){
                //if (!e.has<vot::MMDTag>()){

                auto combMat = rc.baseMat * rc.zmoMat;
                cmd.bindVertexBuffers(rc.vertexBuffer)
                .bindIndexBuffer_(rc.indexBuffer)
                .pushConstants(yic::renderLibrary->GP_Basic.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof (glm::mat4), &combMat);

                for(const auto& [index, subMeshes] : rc.subMeshes){
                    cmd.bindDescriptorSets_(yic::renderLibrary->GP_Basic, rc.dsHandle, index);
                    for(const auto& subMesh : subMeshes){
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
                }
            });

            cmd.bindPipeline_(yic::renderLibrary->GP_Basic_PMX)
            //.bindDescriptorSets_(yic::renderLibrary->GP_Basic_PMX, camera_comp(ecs)->DS);
            .bindDescriptorSets_(yic::renderLibrary->GP_Basic_PMX, ecs.get<sc::Camera>(GLOBAL::camera).DS);

            ecs.view<const vot::RenderVisibleTag, const vot::MMDTag, const vot::RenderComponent>().each([&](entt::entity e, const vot::RenderComponent& rc){
            //ecs.query<const vot::RenderVisibleTag, const vot::MMDTag, const vot::RenderComponent>().each([&](flecs::entity_view e, const vot::RenderVisibleTag, const vot::MMDTag, const vot::RenderComponent& rc){
                auto combMat = rc.baseMat * rc.zmoMat;
                cmd.bindVertexBuffers(rc.vertexBuffer);
                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                cmd.pushConstants(yic::renderLibrary->GP_Basic_PMX.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof (glm::mat4), &combMat);

                for(const auto& [index, subMeshes] : rc.subMeshes){
                    cmd.bindDescriptorSets_(yic::renderLibrary->GP_Basic_PMX, rc.dsHandle, index);
                    for(const auto& subMesh : subMeshes){
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
            });

        });

        // 2
      //  if (*rt.activeImageIndex == 0) {
            yic::renderLibrary->RT_RayTracing->drawRender(cmd, vot::ImageDrawCI()
                    .setOldLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setNewLayout(vk::ImageLayout::eGeneral)
                    .setSrcAccessMask(vk::AccessFlagBits2::eShaderRead)
                    .setDstAccessMask(vk::AccessFlagBits2::eAccelerationStructureWriteKHR)
                    .setSrcStageMask(vk::PipelineStageFlagBits2::eFragmentShader)
                    .setDstStageMask(vk::PipelineStageFlagBits2::eAccelerationStructureBuildKHR)
                    .setSubresourceRange(vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}), [&] {
                if (yic::sceneSystem->acquireActiveScene()->tlas != nullptr) {
                    cmd.bindPipeline_(yic::renderLibrary->RP_Shadow)
                    //.bindDescriptorSets_(yic::renderLibrary->RP_Shadow, camera_comp(ecs)->DS)
                    .bindDescriptorSets_(yic::renderLibrary->RP_Shadow, ecs.get<sc::Camera>(GLOBAL::camera).DS)
                    .bindDescriptorSets_(yic::renderLibrary->RP_Shadow)
                    .traceRaysKHR_(yic::renderLibrary->RP_Shadow, vot::Resolutions::eQHDExtent, 1, ct.dynamicDispatcher);}
            });
      //  }


        iTime += (1.f / 60.f);
        iTime += 1.f / std::max(GLOBAL::fps, 0.01f);;
        yic::renderLibrary->RT_Volumetric_Overcast_Clouds->drawRendering(cmd, [&]{
           cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
           .bindPipeline_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds)
           .pushConstants(yic::renderLibrary->GP_Volumetric_Overcast_Clouds.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (float), &iTime);
           cmd.draw(3, 1, 0, 0);
        });

        // 3
        yic::renderLibrary->RT_Post->drawRendering(cmd, [&]{
            cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
            .bindPipeline_(yic::renderLibrary->GP_Post)
            .bindDescriptorSets_(yic::renderLibrary->GP_Post, *rt.activeImageIndex)
            .draw(3, 1, 0, 0);
        });
    }

    auto RenderSubmissionSystem::frame() -> void {
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