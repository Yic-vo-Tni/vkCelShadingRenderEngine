//
// Created by lenovo on 5/20/2025.
//

#include "RenderLibrary.h"
#include "RHI/Allocator.h"
#include "Editor/ImGuiHub.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "RHI/DescriptorSystem.h"

#include "FastNoiseLite.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

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
        GP_Basic_Assimp.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4(1.f))}))

            .setRenderPass2CI(vot::RenderPass2CI()
            .setColorAttachmentFormats({vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat})
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
                                            rhi::GraphicsPipeline::makeBlendAttachment(),
                                            rhi::GraphicsPipeline::makeBlendAttachment()}))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Basic/model.frag")));

        GP_Basic_PMX.combinePipelineLibrary(GP_Basic_Assimp.acquirePipelineLibrary()
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

        GP_ShadowMap_Assimp.combinePipelineLibrary(vot::PipelineLibrary()
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

        GP_ShadowMap_PMX.combinePipelineLibrary(GP_ShadowMap_Assimp.acquirePipelineLibrary()
            .setVertexInputInterfaceCI(vot::VertexInputInterfaceCI()
            .addVertexInputBindingDescription(0, offsetof(vot::Vertex, boneIds), vk::VertexInputRate::eVertex)
            .addVertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(vot::Vertex, pos))));

        GP_Volumetric_Overcast_Clouds.combinePipelineLibrary(vot::PipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
          //  .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eFragment, 0, sizeof(float)}))

            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Common/screen_triangle.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Atmosphere/volumetric_overcast_clouds.frag")));

        GP_Volumetric_Fog.combinePipelineLibrary(GP_Volumetric_Overcast_Clouds.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addPushConstantRange(vk::PushConstantRange{vk::ShaderStageFlagBits::eFragment, 0, sizeof (glm::mat4 )}))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Fog/volumetric_fog.frag")));

        GP_Post.combinePipelineLibrary(GP_Volumetric_Overcast_Clouds.acquirePipelineLibrary()
            .setPipelineDescriptorSetLayoutCI2(vot::PipelineDescriptorSetLayoutCI2()
            .SET0
            .addDescriptorSetLayoutBinding(1, 0, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 1, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 2, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 3, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 4, vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 5, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment)
            .addDescriptorSetLayoutBinding(1, 6, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment))

            .setRenderPass2CI(vot::RenderPass2CI()
            .setColorAttachmentFormats({vk::Format::eR16G16B16A16Sfloat})
            .setRenderingDepth(vk::True))

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

        RT_GBuffer = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setFormat(vk::Format::eR16G16B16A16Sfloat)
                .setColorAttachmentCount(3)
                .setExtent(RT_RESOLUTION)
                .setDstDepthImageLayout(vk::ImageLayout::eRenderingLocalReadKHR)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Main RT Image");

        RT_ShadowMap = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Shadow RT Image");

        RTX_RayTracing = yic::allocator->allocImage(vot::ImageCI()
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

        RT_Volumetric_Fog = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDynamicRender)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eRenderingLocalReadKHR), "Volumetric fog RT Image");

        RT_Post = yic::allocator->allocImage(vot::ImageCI()
                .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                .updateColorToImGui(vot::uiWidget::eRenderWidget)
                .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                .setFormat(vk::Format::eR16G16B16A16Sfloat)
                .setImageCount(frameImageCount)
                .setExtent(RT_RESOLUTION)
                .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Post RT Image");
    }

    auto RenderLibrary::buildUniqueDSHandle() -> void {
//        FastNoiseLite noise;
//        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2); // 或 Perlin, Cellular等
//        noise.SetFrequency(0.9f); // 控制云结构大小
//        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
//        noise.SetFractalOctaves(5); // fbm迭代次数
//        noise.SetFractalGain(0.5f);
//        noise.SetFractalLacunarity(2.0f);
//
//        const int size = 128;
//        std::vector<float> fbm_volume(size * size * size);
//
//        float max_fbm = -1e9, min_fbm = 1e9;
//        for (int z = 0; z < size; ++z)
//            for (int y = 0; y < size; ++y)
//                for (int x = 0; x < size; ++x) {
//                    // 归一化采样坐标, 控制scale影响细节
//                    float fx = float(x) / size * 10.0f;  // 10.0可调（云团多大）
//                    float fy = float(y) / size * 10.0f;
//                    float fz = float(z) / size * 10.0f;
//                    float val = noise.GetNoise(fx, fy, fz);
//                    max_fbm = std::max(max_fbm, val);
//                    min_fbm = std::min(min_fbm, val);
//                    fbm_volume[z * size * size + y * size + x] = val;
//                }
//
//        for (auto& v : fbm_volume)
//            v = (v - min_fbm) / (max_fbm - min_fbm);
//        std::vector<uint8_t> out(size*size);
//        int z = 64; // 切一层
//        for(int y=0; y<size; ++y)
//            for(int x=0; x<size; ++x)
//                out[y*size+x] = uint8_t(255.0f * fbm_volume[z*size*size + y*size + x]);
//        stbi_write_png("fbm_check.png", size, size, 1, out.data(), size);
//
//        T_fbmNoise = yic::allocator->allocImage(vot::ImageCI()
//                .setExtent(vk::Extent3D{size, size, size})
//                .setImageCount(1)
//                .setFormat(vk::Format::eR32Sfloat)
//                .setImageType(vk::ImageType::e3D)
//                .setImageViewType(vk::ImageViewType::e3D)
//                .setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst), "fbm noise"
//                );
//
//        yic::allocator->uploadImage(T_fbmNoise->images, fbm_volume.data(), vk::Extent3D{size, size, size}, vk::Format::eR16Sfloat);
//        yic::logger->info("T_fbm ok");

        T_blueNoise64 = yic::allocator->loadTexture(R"(H:\VkCelShadingRenderer\renderer\resource\Texture\LDR_LLL1_0.png)");

//        GP_Volumetric_Overcast_Clouds.DS = yic::desSystem->allocUpdateDescriptorSets([&]{
//            vot::DescriptorLayout2 layout{};
//
//            for (auto i = 0u; i < frameImageCount; i++) {
//                auto base = RT_GBuffer->config.colorAttachmentCount * i;
//                layout.emplace(vot::DescriptorLayout2::_1d{
//                      T_fbmNoise->imageInfo(),
//                });
//            }
//
//            return layout;
//        }, GP_Volumetric_Overcast_Clouds);

        GP_Volumetric_Fog.DS = yic::desSystem->allocUpdateDescriptorSets([&] {
            vot::DescriptorLayout2 layout{};

            for (auto i = 0u; i < frameImageCount; i++) {
                auto base = RT_GBuffer->config.colorAttachmentCount * i;
                layout.emplace(vot::DescriptorLayout2::_1d{
                        RT_GBuffer->imageInfo(base + eGBuffer::ePosition, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_ShadowMap->imageInfo(i),
                        T_blueNoise64->imageInfo(),
                });
            }

            return layout;
        }, GP_Volumetric_Fog);

        GP_Post.DS = yic::desSystem->allocUpdateDescriptorSets([&]{
            vot::DescriptorLayout2 layout{};

            for(auto i = 0u; i < frameImageCount; i++){
                auto base = RT_GBuffer->config.colorAttachmentCount * i;
                layout.emplace(vot::DescriptorLayout2::_1d {
                        RT_GBuffer->imageInfo(base + eGBuffer::eAlbedo, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_GBuffer->imageInfo(base + eGBuffer::ePosition, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_GBuffer->imageInfo(base + eGBuffer::eNormal, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_Volumetric_Fog->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RT_Volumetric_Clouds->imageInfo(i, std::nullopt, vk::ImageLayout::eRenderingLocalReadKHR),
                        RTX_RayTracing->imageInfo(),
                        T_blueNoise64->imageInfo(),
                });
            }

            return layout;
        }, GP_Post);
    }

} // sc


