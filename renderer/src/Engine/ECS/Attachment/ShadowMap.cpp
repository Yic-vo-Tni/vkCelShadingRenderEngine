//
// Created by lenovo on 9/17/2024.
//

#include "ShadowMap.h"
#include "Engine/RHI/Allocator.h"
#include "Engine/RHI/FrameRender.h"

namespace sc {
    ShadowMap::ShadowMap() {
        ecs = mg::SystemHub.val<ev::pEcs>().ecs;
        build();
    }

    auto ShadowMap::build() -> void {
        mShadowMap = mg::Allocator->allocImage(yic2::ImageConfig()
                .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment)
                .setFlags(yic2::ImageFlagBits::eDepthStencil | yic2::ImageFlagBits::eDynamicRender)
                .setExtent(mExtent)
                .setImageCount(3));

        mGraphicsGroup = yic::RenderGroupGraphics ::configure(VK_NULL_HANDLE)
                ->addPushConstantRange_(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4))
                ->addBindingDescription_(0, sizeof(sc::Vertex), vk::VertexInputRate::eVertex)
                ->addAttributeDescription_(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(sc::Vertex, pos))
                ->addShader_("ShadowMap/v.vert", vk::ShaderStageFlagBits::eVertex)
                ->addShader_("ShadowMap/f.frag", vk::ShaderStageFlagBits::eFragment)
                ->build();
    }

    auto ShadowMap::render(vk::CommandBuffer &cmd, const vot::aabb& aabb, glm::vec3 lightDir) -> void {
        glm::mat4 view = glm::lookAt(aabb.center() - glm::normalize(lightDir) * aabb.distance(), center, up);
        auto fix = (aabb.max - aabb.min) * 0.1f;
        glm::mat4 proj = glm::ortho(aabb.min.x - fix.x, aabb.max.x + fix.x, aabb.min.y - fix.y, aabb.max.y + fix.y, 0.1f, aabb.distance() * 2);
        glm::mat4 lightMvp = proj * view;

//        auto index = *mg::SystemHub.val<ev::hVkRenderContext>(toolkit::enum_name(RenderPhase::ePrimary)).activeImageIndex;
//        auto dispatcher = *mg::SystemHub.val<ev::pVkSetupContext>().dynamicDispatcher;
//
//        auto colorAttach = vk::RenderingAttachmentInfo()
//                .setImageView(mShadowMap->imageViews[index])
//                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
//                .setLoadOp(vk::AttachmentLoadOp::eClear)
//                .setStoreOp(vk::AttachmentStoreOp::eStore)
//                .setClearValue(vk::ClearColorValue{0.f, 0.f, 0.f, 0.f});
//        auto depthStencilAttach = vk::RenderingAttachmentInfo()
//                .setImageView(mShadowMap->depthImageView)
//                .setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
//                .setLoadOp(vk::AttachmentLoadOp::eClear)
//                .setStoreOp(vk::AttachmentStoreOp::eStore)
//                .setClearValue(vk::ClearDepthStencilValue{1.f, 0});
//
//        auto renderingInfo = vk::RenderingInfo()
//                .setRenderArea(vk::Rect2D{{0, 0}, {2560, 1440}})
//                .setLayerCount(1)
//                .setColorAttachments(colorAttach)
//                .setPDepthAttachment(&depthStencilAttach)
//                .setPStencilAttachment(&depthStencilAttach);
//
//        cmd.beginRendering(renderingInfo, dispatcher);
//
//        cmd.pushConstants(mGraphicsGroup->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &lightMvp);
//        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsGroup->acquire());
//        ecs->query<Model>().each([&](flecs::entity e, Model& model) {
//            for (const auto &subMesh: model.mesh.subMeshes) {
//                cmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
//                cmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
//                cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
//            }
//        });
//        cmd.endRendering(dispatcher);


        mShadowMap->dynamicRendering(cmd, [&]{
           cmd.pushConstants(mGraphicsGroup->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &lightMvp);
           cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsGroup->acquire());
           ecs->query<Model>().each([&](flecs::entity e, Model& model){
               cmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
               cmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
               for (const auto &subMesh: model.mesh.subMeshes) {
                   cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
               }
           });
        });
    }
} // sc