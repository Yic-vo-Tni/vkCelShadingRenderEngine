//
// Created by lenovo on 5/20/2025.
//

#include "RenderLibrary.h"
#include "RHI/Allocator.h"
#include "Editor/ImGuiHub.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/DescriptorSystem.h"

namespace sc {

#define SET0  addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR)

    RenderLibrary::RenderLibrary() {
        frameImageCount = yic::systemHub.val<ev::pVkRenderContext>().frameEntries->size();
        buildPipelines();
        buildRenderTarget();
        buildUniqueDSHandle();
    }

    RenderLibrary::~RenderLibrary() = default;

    auto RenderLibrary::buildPipelines() -> void {
        auto format = yic::systemHub.val<ev::pVkRenderContext>().surfaceFormat->format;

        GP_Basic.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4(1.f))}))

            .setRenderPass2CI(vot::RenderPass2CI()
            .setColorAttachmentFormats({format, format})
            .setRenderingDepth(vk::True))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, sizeof(vot::Vertex), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))
            .addVertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, nor))
            .addVertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vot::Vertex, uv))
            .addVertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32A32Sint, offsetof(vot::Vertex, boneIds))
            .addVertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vot::Vertex, boneWeight)))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Basic/model.vert"))

            .setFragmentOutputInterfaceCI(vot::FragmentOutputInterfaceCI()
            .setColorBlendAttachmentStates({rhi::GraphicsPipeline::makeBlendAttachment(),
                                            rhi::GraphicsPipeline::makeBlendAttachment()}))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/model.frag")));

        GP_Basic_PMX.combinePipelineLibrary(GP_Basic.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4(1.f))}))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, offsetof(vot::Vertex, boneIds), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))
            .addVertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, nor))
            .addVertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vot::Vertex, uv)))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Basic/pmx.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/pmx.frag")));

        GP_ShadowMap_Basic.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof (glm::mat4 )}))

            .setRenderPass2CI(vot::RenderPass2CI()
            .setRenderingDepth(vk::True))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, sizeof(vot::Vertex), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos)))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("ShadowMap/DirectionLight.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("ShadowMap/DirectionLight.frag")));

        GP_ShadowMap_PMX.combinePipelineLibrary(GP_ShadowMap_Basic.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof (glm::mat4 )})) //

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, offsetof(vot::Vertex, boneIds), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))));

        GP_Volumetric_Overcast_Clouds.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eFragment, 0, sizeof(float)}))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI())

            .setRenderPass2CI(vot::RenderPass2CI()
            .setRenderingDepth(vk::True))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Atmosphere/volumetric_overcast_clouds.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Atmosphere/volumetric_overcast_clouds.frag")));

        GP_Post.combinePipelineLibrary(GP_Volumetric_Overcast_Clouds.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 2, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 3, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI())

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Basic/post.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/post.frag")));

       RP_Shadow.addShader("RT/gen.rgen", vk::ShaderStageFlagBits::eRaygenKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
            .addShader("RT/miss.rmiss", vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
            .addShader("RT/shadow_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
            .addShader("RT/hit.rchit", vk::ShaderStageFlagBits::eClosestHitKHR, vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, vot::RTShaderRole::eClosestHit)

            .build(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR)
            .addDescriptorSetLayoutBinding(1, 2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR));

    }

    auto RenderLibrary::buildRenderTarget() -> void {
        auto RT_RESOLUTION = vot::Resolutions::eQHDExtent;

        RT_Main = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setColorAttachmentCount(2) // albedo pos
                .setExtent(RT_RESOLUTION)
                .setDstDepthImageLayout(vk::ImageLayout::eRenderingLocalReadKHR)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Main RT Image");

        RT_ShadowMap = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .updateColorToImGui(vot::uiWidget::eViewWidget)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Shadow RT Image");

        RT_RayTracing = yic::allocator->allocImage(vot::ImageCI()
                .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
                .setImageCount(1)
                .setExtent(RT_RESOLUTION)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "RayTracing RT Image");

        RT_Volumetric_Clouds = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Volumetric overcast clouds RT Image");

        RT_Post = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .updateColorToImGui(vot::uiWidget::eRenderWidget)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Post RT Image");
    }

    auto RenderLibrary::buildUniqueDSHandle() -> void {
        GP_Post.DS = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};

            for(auto i = 0u; i < frameImageCount; i++){
                auto base = RT_Main->config.colorAttachmentCount * i;
                layout.emplace(vot::DescriptorLayout2::_1d {
                        RT_Main->imageInfo(base, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_Main->imageInfo(base + 1, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_Volumetric_Clouds->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_RayTracing->imageInfo(),
                });
            }

            return layout;
        }, GP_Post);
    }

} // sc


