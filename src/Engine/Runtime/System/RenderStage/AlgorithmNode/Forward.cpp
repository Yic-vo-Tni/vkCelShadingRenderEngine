//
// Created by lenovo on 1/8/2026.
//

#include "Forward.h"
#include "Editor/ImGuiHub.h"
#include "RHI/Allocator.h"
#include "Runtime/System/DER/DynamicEditableRendering.h"

namespace runtime::flow {
    auto Compose_Target() -> vot::Image_sptr {
        return yic::allocator->allocImage(vot::ImageCI()
                                          .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
                                          .updateColorToImGui(vot::uiWidget::eRenderWidget)
                                          .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
                                          .setFormat(vk::Format::eR8G8B8A8Unorm)
                                          .setImageCount(3)
                                          .setExtent(vot::Resolutions::eQHDExtent)
                                          .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal), "Compose");
    }

    auto Compose_Pipline() -> PipelineHandle {
        const auto pipeline = std::make_shared<rhi::GraphicsPipeline>();
        pipeline->combinePipelineLibrary(vot::PipelineLibrary()
            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
            .setShaderPath("Common/screen_triangle.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
            .setShaderPath("Common/dummy.frag")));
        return std::move(pipeline);
    }

    auto Compose_Descriptor() -> vot::DescriptorHandle {
        return {};
    }

    auto Compose_Dispatch(vot::CommandBuffer &cmd) -> void {
        auto& compose_pipe = yic::derTranslator->pipeline<rhi::GraphicsPipeline>("Compose");
        cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
        cmd.bindPipeline_(*compose_pipe);
        cmd.draw(3, 1, 0, 0);
    }

} // runtime