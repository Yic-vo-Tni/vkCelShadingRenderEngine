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
                .setFlags(yic2::ImageFlagBits::eDepthStencil)
                .setExtent(mExtent)
                .setImageCount(3));

        mGraphicsGroup = yic::RenderGroupGraphics ::configure(yic::FrameRender::eColorDepthStencilRenderPass)
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

        cmd.pushConstants(mGraphicsGroup->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4), &lightMvp);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsGroup->acquire());
        ecs->query<Model>().each([&](flecs::entity e, Model& model) {
            for (const auto &subMesh: model.mesh.subMeshes) {
                cmd.bindVertexBuffers(0, model.mesh.vertBuf->buffer, {0});
                cmd.bindIndexBuffer(model.mesh.indexBuf->buffer, 0, vk::IndexType::eUint32);
                cmd.drawIndexed(subMesh.indexCount, 1, subMesh.firstIndex, 0, 0);
            }
        });
    }
} // sc