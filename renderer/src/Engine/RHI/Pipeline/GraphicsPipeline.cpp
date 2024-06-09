//
// Created by lenovo on 6/9/2024.
//

#include "GraphicsPipeline.h"

namespace yic {

    Graphics::Graphics(vk::Device device, vk::PipelineLayout layout, vk::RenderPass renderPass) : mDevice(device) {
        inputAssemblyState.setTopology(vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable({});

        vertexInputState.setVertexAttributeDescriptions({})
                .setVertexBindingDescriptions({});

        dynamicState.setDynamicStates({});

        viewportState.setViewports({})
                .setScissors({});

        rasterizationState.setDepthClampEnable({})
                .setRasterizerDiscardEnable({})
                .setPolygonMode(vk::PolygonMode::eFill)
                .setFrontFace(vk::FrontFace::eClockwise)
                .setDepthBiasEnable({})
                .setDepthBiasConstantFactor({})
                .setDepthBiasClamp({})
                .setDepthBiasSlopeFactor({})
                .setLineWidth(1.f);

        multisampleState.setRasterizationSamples(vk::SampleCountFlagBits::e1);

        depthStencilState.setDepthTestEnable(vk::True)
                .setDepthWriteEnable(vk::True)
                .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                .setDepthBoundsTestEnable({})
                .setStencilTestEnable({})
                .setFront(vk::StencilOpState{})
                .setBack(vk::StencilOpState{})
                .setMinDepthBounds({})
                .setMaxDepthBounds({});

        colorBlendState.setLogicOpEnable({})
                .setAttachments({});

        createInfo.setPInputAssemblyState(&inputAssemblyState)
                .setPVertexInputState(&vertexInputState)
                .setPDynamicState(&dynamicState)
                .setPViewportState(&viewportState)
                .setPRasterizationState(&rasterizationState)
                .setPMultisampleState(&multisampleState)
                .setPDepthStencilState(&depthStencilState)
                .setPColorBlendState(&colorBlendState)
                .setLayout(layout)
                .setRenderPass(renderPass);
    }

    void Graphics::updateState() {
        if (blendAttachmentStates.empty()){
            blendAttachmentStates.emplace_back(makePipelineColorBlendAttachments());
            colorBlendState.setAttachments(blendAttachmentStates);
        }
        colorBlendState.setAttachments(blendAttachmentStates);

        dynamicState.setDynamicStates(dynamicStates);

        vertexInputState.setVertexAttributeDescriptions(attributeDescriptions)
                .setVertexBindingDescriptions(bindingDescriptions);

        if(viewports.empty()){
            viewportState.setViewportCount(1);
        } else{
            viewportState.setViewports(viewports);
        }

        if (scissors.empty()){
            viewportState.setScissorCount(1);
        } else{
            viewportState.setScissors(scissors);
        }
    }

} // yic