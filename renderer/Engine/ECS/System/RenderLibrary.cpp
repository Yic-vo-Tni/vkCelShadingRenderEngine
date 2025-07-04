//
// Created by lenovo on 5/20/2025.
//

#include "RenderLibrary.h"
#include "RHI/Allocator.h"
#include "Editor/ImGuiHub.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/DescriptorSystem.h"

namespace sc {

#define SET0  addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eRaygenKHR)

    RenderLibrary::RenderLibrary() {
        frameImageCount = yic::systemHub.val<ev::pVkRenderContext>().frameEntries->size();
        buildPipelines();
        buildRenderTarget();
        buildUniqueDSHandle();
    }

    RenderLibrary::~RenderLibrary() = default;

    auto RenderLibrary::buildPipelines() -> void {
        GP_Basic.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4(1.f))}))

            .setRenderPass2CI(vot::RenderPass2CI()
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

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/model.frag")));

        GP_Basic_PMX.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4(1.f))}))

            .setRenderPass2CI(vot::RenderPass2CI()
            .setRenderingDepth(vk::True))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, offsetof(vot::Vertex, boneIds), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))
            .addVertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, nor))
            .addVertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vot::Vertex, uv)))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Basic/pmx.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/pmx.frag")));

        GP_Volumetric_Overcast_Clouds.combinePipelineLibrary(GP_Basic.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eFragment, 0, sizeof(float)}))

            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI())

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Atmosphere/volumetric_overcast_clouds.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Atmosphere/volumetric_overcast_clouds.frag")));

        GP_Post.combinePipelineLibrary(GP_Basic.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment))

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
        RT_Main = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(vot::Resolutions::eQHDExtent)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Main RT Image");

        RT_RayTracing = yic::allocator->allocImage(vot::ImageCI()
                .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
                .setImageCount(1)
                .setExtent(vot::Resolutions::eQHDExtent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "RayTracing RT Image");

        RT_Volumetric_Overcast_Clouds = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
//                .updateColorToImGui(vot::uiWidget::eViewWidget)
                .setImageCount(frameImageCount)
                .setExtent(vot::Resolutions::eQHDExtent)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Volumetric overcast clouds RT Image");

        RT_Post = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .updateColorToImGui(vot::uiWidget::eRenderWidget)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(vot::Resolutions::eQHDExtent)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Post RT Image");
    }

    auto RenderLibrary::buildUniqueDSHandle() -> void {
        GP_Post.DS = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};

            for(auto i = 0u; i < frameImageCount; i++){
                layout.emplace(vot::DescriptorLayout2::_1d {
                        RT_Main->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_Volumetric_Overcast_Clouds->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_RayTracing->imageInfo(),
                });
            }

            return layout;
        }, GP_Post);
    }

} // sc




//        DS_GP_Post = rhi::Descriptor::make_shared()->combine(yic::renderLibrary->GP_Post.acquirePipelineLibrary())
//                ->updateDescriptorSets([&]() {
//                    rhi::DescriptorLayout layout{};
//
//                    for(auto i = 0; i < frameImageCount; i++){
//                        layout.emplace(rhi::DescriptorLayout ::_1d {
//                                yic::renderLibrary->RT_Main->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
//                                yic::renderLibrary->RT_RayTracing->imageInfo(),
//                        });
//                    }
//
//                    return layout;
//                });

//        RP_Shadow.addShader("RT/gen.rgen", vk::ShaderStageFlagBits::eRaygenKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
//            .addShader("RT/miss.rmiss", vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
//            .addShader("RT/shadow_miss.rmiss", vk::ShaderStageFlagBits::eMissKHR, vk::RayTracingShaderGroupTypeKHR::eGeneral)
//            .addShader("RT/hit.rchit", vk::ShaderStageFlagBits::eClosestHitKHR, vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, vot::RTShaderRole::eClosestHit)
//
//            .build(vot::PipelineDescriptorSetLayoutCI()
//            .addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eAccelerationStructureKHR, vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR)
//            .addDescriptorSetLayoutBinding(0, 1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eRaygenKHR)
//            .addDescriptorSetLayoutBinding(0, 2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eRaygenKHR)
//            .addDescriptorSetLayoutBinding(0, 3, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR));

//        GP_Basic.combinePipelineLibrary(vot::PipelineLibrary()
//            .setPipelineDescriptorSetLayoutCI(vot::PipelineDescriptorSetLayoutCI()
//            .addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
//            .addDescriptorSetLayoutBinding(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
//            .addDescriptorSetLayoutBinding(0, 2, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex))
//
//            .setRenderPass2CI(vot::RenderPass2CI()
//            .setRenderingDepth(vk::True))
//
//            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
//            .addVertexInputBindingDescription(0, sizeof(vot::Vertex), vk::VertexInputRate::eVertex)
//            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))
//            .addVertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, nor))
//            .addVertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vot::Vertex, uv))
//            .addVertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32A32Sint, offsetof(vot::Vertex, boneIds))
//            .addVertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(vot::Vertex, boneWeight)))
//
//            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
//            .setShaderPath("Basic/model.vert"))
//
//            .setFragmentShaderCI(vot::FragmentShaderCI()
//            .setShaderPath("Basic/model.frag")));

//        GP_Outline.combinePipelineLibrary(vot::PipelineLibrary()
//                                                  .setPipelineDescriptorSetLayoutCI(vot::PipelineDescriptorSetLayoutCI()
//                                                  .addDescriptorSetLayoutBinding(0, 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
//                                                  .addDescriptorSetLayoutBinding(0, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment))
//
//                                                  .setRenderPass2CI(vot::RenderPass2CI()
//                                                  .setRenderingDepth(vk::True))
//
//                                                  .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
//                                                  .setPrimitiveTopology(vk::PrimitiveTopology::eTriangleListWithAdjacency)
//                                                  .addVertexInputBindingDescription(0, sizeof(vot::Vertex), vk::VertexInputRate::eVertex)
//                                                  .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))
//                                                  .addVertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, nor))
//                                                  .addVertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(vot::Vertex, uv)))
//
//                                                  .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
//                                                  .setDepthBiasEnable(vk::True)
//                                                  .setDepthBiasConstantFactor(1.f)
//                                                  .setDepthBiasSlopeFactor(1.f)
//                                                  .setShaderPath("Basic/outline.vert")
//                                                  .setGeomShaderPath("Basic/outline.geom"))
//
//                                                  .setFragmentShaderCI(vot::FragmentShaderCI()
//                                                  .setShaderPath("Basic/outline.frag")));