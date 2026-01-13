//
// Created by lenovo on 1/8/2026.
//


#include "Editor/ImGuiHub.h"
#include "Runtime/System/DER/DynamicEditableRendering.h"

extern "C" {

__declspec(dllexport)
vot::ImageCI Compose_Target() {
    return vot::ImageCI()
            .setFlags(vot::imageFlagBits::eDepthStencil | vot::imageFlagBits::eDynamicRender)
            .updateColorToImGui(vot::uiWidget::eRenderWidget)
            .addUsage(vk::ImageUsageFlagBits::eInputAttachment)
            .setFormat(vk::Format::eR8G8B8A8Unorm)
            .setImageCount(3)
            .setExtent(vot::Resolutions::eQHDExtent)
            .setDstImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}


__declspec(dllexport)
vot::PipelineLibrary Compose_Pipeline() {
    return vot::PipelineLibrary()
            .setPreRasterizationShadersCI(vot::PreRasterizationShadersCI()
                .setShaderPath("Common/screen_triangle.vert"))

            .setFragmentShaderCI(vot::FragmentShaderCI()
                .setShaderPath("Common/dummy.frag"));
}


__declspec(dllexport)
vot::DescriptorHandle Compose_Descriptor() {
    return {};
}

__declspec(dllexport)
void Compose_Dispatch(vot::CommandBuffer &cmd, const runtime::flow::DERTranslator *tsr) {
    auto &compose_pipe = tsr->pipeline<rhi::GraphicsPipeline>("Compose");
    cmd.setRenderArea_(vot::Resolutions::eQHDExtent);
    cmd.bindPipeline_(*compose_pipe);
    cmd.draw(3, 1, 0, 0);
}
}
