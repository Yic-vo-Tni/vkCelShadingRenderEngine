//
// Created by lenovo on 10/16/2025.
//

#include "RenderStage.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Core/Management/TripleBufferIndexManager.h"
#include "Runtime/System/RenderLibrary.h"
#include "RS/ResourceSystem.h"
#include "Runtime/Camera/Camera.h"
#include "SM/Scene.h"
#include "SM/illuminate/DirectionLight.h"

namespace sc {
    RenderStage::RenderStage(entt::registry &registry) : ecs(registry) {
        ct = yic::systemHub.va<ev::pVkSetupContext>();
        rt = yic::systemHub.va<ev::pVkRenderContext>();
    }

    auto RenderStage::update() -> void {
        iTime += (1.f / 60.f);
        iTime += 1.f / std::max(GLOBAL::fps, 0.01f);;
        fast = yic::indexRing.get(vot::LogicBufferType::eFast).render_cur();
        slow = yic::indexRing.get(vot::LogicBufferType::eSlow).render_cur();
        set0 = GLOBAL::entity::set0.va<vot::DescriptorSet0>().handles[fast];
    }

    auto RenderStage::comp_skinning(vot::CommandBuffer &cmd) const -> void {
        ecs.view<const vot::mark::eVisible, const vot::mark::eAssimp, const vot::RenderComponent, const vot::VertexDataComponent, const vot::AnimationComponent>()
        .each([&](entt::entity, const vot::RenderComponent &rc, const vot::VertexDataComponent& vdc, const vot::AnimationComponent& ac) {
            const uint32_t max_id = vdc.vertices_pmr[vot::VertexDataComponent::eAnim].size();
            constexpr auto localSize = 64u;
            const auto groupCount = (vdc.vertices_pmr[0].size() + localSize - 1) / localSize;

            ac.boneMatBuffer->update(ac.boneMats[slow]);
            cmd.bindPipeline_(yic::renderLibrary->CP_Skinning);
            cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, yic::renderLibrary->CP_Skinning.acquirePipelineLayout(), 1, rc.dsHandle.va(), {});
            cmd.pushConstants(yic::renderLibrary->CP_Skinning.acquirePipelineLayout(), vk::ShaderStageFlagBits::eCompute, 0, sizeof (std::uint32_t), &max_id);
            cmd.dispatch(groupCount, 1, 1);
        });
    }

    auto RenderStage::drawing_gBuffer(vot::CommandBuffer &cmd) -> void {
        auto draw_meshes = [&](rhi::GraphicsPipeline& pipe, const uint32_t slot, auto view) {
            cmd.bindPipeline_(pipe)
            .bindDescriptorSets_(pipe, set0);

            view.each([&](entt::entity, const vot::RenderComponent& rc) {
                const auto combMat = rc.baseMat * rc.zmoMat;
                cmd.bindVertexBuffers(rc.vertexBuffer[slot]);
                cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                cmd.pushConstants(pipe.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &combMat);

                for (const auto& [index, subMeshes]: rc.subMeshes) {
                    cmd.bindDescriptorSets_(pipe, rc.dsHandle, index);
                    for (const auto &subMesh : subMeshes) {
                        cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
                    }
                }
            });
        };

        auto draw_light = [&] {
            cmd.bindPipeline_(yic::renderLibrary->GP_Light)
            .bindDescriptorSets_(yic::renderLibrary->GP_Light, set0);

            ecs.view<const vot::mark::eVisible, const vot::RenderComponent, const vot::comp::Light::Meta>()
                    .each([&](entt::entity, const vot::RenderComponent &rc, const vot::comp::Light::Meta &meta) {
                        const auto combMat = rc.baseMat * rc.zmoMat;
                        cmd.bindVertexBuffers(rc.vertexBuffer[vot::VertexDataComponent::eAnim]);
                        cmd.bindIndexBuffer(rc.indexBuffer->buffer, 0, rc.indexType);
                        cmd.pushConstants(yic::renderLibrary->GP_Light.acquirePipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &combMat);

                        for (auto i = 0u; i < meta.usedCount(); i++) {
                            for (const auto &subMeshes: rc.subMeshes | std::views::values) {
                                for (const auto &[indexCount, firstIndex]: subMeshes) {
                                    cmd.drawIndexed(indexCount, 1, firstIndex, 0, 0);
                                }
                            }
                        }
                    });
        };

        cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
        draw_meshes(yic::renderLibrary->GP_Basic_Assimp, vot::VertexDataComponent::eAnim, ecs.view<const vot::mark::eVisible, const vot::mark::eAssimp, const vot::RenderComponent>());
        draw_meshes(yic::renderLibrary->GP_Basic_PMX, slow, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());
        draw_light();

    }

    auto RenderStage::drawing_shadowMap(vot::CommandBuffer &cmd) -> void {
        auto draw_meshes_shadowMap = [&](rhi::GraphicsPipeline& pipeline, auto view) {
            //auto cam = ecs.get<sc::Camera>(GLOBAL::camera);
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

        cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
        draw_meshes_shadowMap(yic::renderLibrary->GP_ShadowMap_Assimp, ecs.view<const vot::mark::eVisible, const vot::mark::eAssimp, const vot::RenderComponent>());
        draw_meshes_shadowMap(yic::renderLibrary->GP_ShadowMap_PMX, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());}

    auto RenderStage::drawing_IDBuffer(vot::CommandBuffer &cmd) -> void {
        auto fn = [&](rhi::GraphicsPipeline& pipeline, auto view) {
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

        fn(yic::renderLibrary->GP_IDBuffer_Assimp, ecs.view<const vot::mark::eVisible, const vot::mark::eAssimp, const vot::RenderComponent>());
        fn(yic::renderLibrary->GP_IDBuffer, ecs.view<const vot::mark::eVisible, const vot::mark::eMMD, const vot::RenderComponent>());
    }

    auto RenderStage::drawing_volumetricClouds(vot::CommandBuffer &cmd) const -> void {
        cmd.setRenderArea_(vot::Resolutions::eFullHDExtent);
        if (GLOBAL::showVolumetricClouds) {
            constexpr vk::FragmentShadingRateCombinerOpKHR vrsCombiner[] = {
                vk::FragmentShadingRateCombinerOpKHR::eReplace,
                vk::FragmentShadingRateCombinerOpKHR::eReplace,
            };

            cmd.bindPipeline_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds)
            .bindDescriptorSets_(yic::renderLibrary->GP_Volumetric_Overcast_Clouds, set0)
            .pushConstants(yic::renderLibrary->GP_Volumetric_Overcast_Clouds.acquirePipelineLayout(), vk::ShaderStageFlagBits::eFragment, 0, sizeof (float), &iTime);
            // cmd.setFragmentShadingRateKHR(vk::Extent2D{2, 2}, vrsCombiner, *ct.dynamicDispatcher);
            cmd.draw(3, 1, 0, 0);
        } else {
            cmd.bindPipeline_(yic::renderLibrary->GP_Dummy);
            cmd.draw(3, 1, 0, 0);
        }
    }

    auto RenderStage::drawing_volumetricFog(vot::CommandBuffer &cmd) const -> void {
        if (!GLOBAL::showVolumetricFog) {
            const auto cam = GLOBAL::entity::camera.va<Camera>();
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
    }

    auto RenderStage::drawing_post(vot::CommandBuffer &cmd) const -> void {
        cmd.setRenderArea_(vot::Resolutions::eQHDExtent)
          .bindDescriptorSets_(yic::renderLibrary->GP_Post, set0)
          .bindPipeline_(yic::renderLibrary->GP_Post)
          .bindDescriptorSets_(yic::renderLibrary->GP_Post, *rt.activeImageIndex)
          .draw(3, 1, 0, 0);
    }

    auto RenderStage::draw_RTShadow(vot::CommandBuffer &cmd) const -> void {
        if (yic::sceneSystem->acquireActiveScene()->tlas != nullptr) {
            cmd.bindPipeline_(yic::renderLibrary->RP_Shadow)
            .bindDescriptorSets_(yic::renderLibrary->RP_Shadow, set0)
            .bindDescriptorSets_(yic::renderLibrary->RP_Shadow)
            .traceRaysKHR_(yic::renderLibrary->RP_Shadow, vot::Resolutions::eQHDExtent, 1, ct.dynamicDispatcher);
        }
    }
} // sc