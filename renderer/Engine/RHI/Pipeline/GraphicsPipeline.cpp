//
// Created by lenovo on 9/26/2024.
//

#include "GraphicsPipeline.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Utils/FileOperation.h"

#include "Editor/ShaderHotReload/ShaderHotReload.h"

namespace rhi {

    GraphicsPipeline::GraphicsPipeline() : ct(yic::systemHub.val<ev::pVkSetupContext>()) {

    }

    GraphicsPipeline::~GraphicsPipeline() {
//        mPipelineLibrary.pipelineDescriptorSetLayoutCI.clear();
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>)
                arg.clear();
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>)
                arg.clear(ct.device);
        }, mPipelineLibrary.pipelineDescriptorSetLayoutCI);
        if (mPipelineLibrary.pipelineLayout) ct.device->destroy(mPipelineLibrary.pipelineLayout);
        if (mPipelineLibrary.renderPass) ct.device->destroy(mPipelineLibrary.renderPass);
        if (mPipelineLibrary.vertexInputInterface) ct.device->destroy(mPipelineLibrary.vertexInputInterface);
        if (mPipelineLibrary.preRasterizationShaders) ct.device->destroy(mPipelineLibrary.preRasterizationShaders);
        if (mPipelineLibrary.fragmentOutputInterface) ct.device->destroy(mPipelineLibrary.fragmentOutputInterface);
        if (mPipelineLibrary.fragmentShader) ct.device->destroy(mPipelineLibrary.fragmentShader);
        if (mFinalPipeline) ct.device->destroy(mFinalPipeline);
    }

    auto GraphicsPipeline::combinePipelineLibrary(vot::PipelineLibrary pipelineLibrary) -> void {
        if (!pipelineLibrary.pipelineLayout) buildPipelineLayout(pipelineLibrary);
        if (!pipelineLibrary.renderPass) buildRenderPass(pipelineLibrary);

        mPipelineLibrary = std::move(pipelineLibrary);

        if (!mPipelineLibrary.vertexInputInterface) buildVertexInputInterfaceLibrary();
        if (!mPipelineLibrary.preRasterizationShaders) buildPreRasterizationShadersLibrary();
        if (!mPipelineLibrary.fragmentOutputInterface) buildFragmentOutputInterfaceLibrary();
        if (!mPipelineLibrary.fragmentShader) buildFragmentShaderLibrary();

//        auto libraries = {
//                mPipelineLibrary.vertexInputInterface,
//                mPipelineLibrary.preRasterizationShaders,
//                mPipelineLibrary.fragmentOutputInterface,
//                mPipelineLibrary.fragmentShader
//        };
//        auto libraryCI = vk::PipelineLibraryCreateInfoKHR()
//                .setLibraries(libraries);
//
//        if (mFinalPipeline) ct.device->destroy(mFinalPipeline);
//        mFinalPipeline = vot::create("create pipeline") = [&]{
//            return ct.device->createGraphicsPipeline(mPipelineCache, vk::GraphicsPipelineCreateInfo()
//                    .setPNext(&libraryCI)
//                    .setLayout(mPipelineLibrary.pipelineLayout)).value;
//        };
        build();
    }

    auto GraphicsPipeline::build() -> void {
        auto libraries = {
                mPipelineLibrary.vertexInputInterface,
                mPipelineLibrary.preRasterizationShaders,
                mPipelineLibrary.fragmentOutputInterface,
                mPipelineLibrary.fragmentShader
        };
        auto libraryCI = vk::PipelineLibraryCreateInfoKHR()
                .setLibraries(libraries);

        if (mFinalPipeline) ct.device->destroy(mFinalPipeline);
        mFinalPipeline = vot::create("create pipeline") = [&]{
            return ct.device->createGraphicsPipeline(mPipelineCache, vk::GraphicsPipelineCreateInfo()
                    .setPNext(&libraryCI)
                    .setLayout(mPipelineLibrary.pipelineLayout)).value;
        };
    }

    auto GraphicsPipeline::buildVertexInputInterfaceLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.vertexInputInterfaceCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eVertexInputInterface);

        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{{},
                                                                    libraryCI.primitiveTopology.value_or(vk::PrimitiveTopology::eTriangleList),
                                                                    {}};

        auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
                .setVertexBindingDescriptions(libraryCI.vertexInputBindings)
                .setVertexAttributeDescriptions(libraryCI.vertexInputAttributes);

        auto ci = vk::GraphicsPipelineCreateInfo()
                .setFlags(vk::PipelineCreateFlagBits::eLibraryKHR |
                          vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT)
                .setPInputAssemblyState(&inputAssemblyState)
                .setPVertexInputState(&vertexInputState)
                .setPNext(&libraryInfo);

        mPipelineLibrary.vertexInputInterface = ct.device->createGraphicsPipeline(mPipelineCache, ci, nullptr).value;
    }

    auto GraphicsPipeline::buildPreRasterizationShadersLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.preRasterizationShadersCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::ePreRasterizationShaders);

        auto rasterizationState = vk::PipelineRasterizationStateCreateInfo()
                .setDepthClampEnable({})
                .setRasterizerDiscardEnable({})
                .setPolygonMode(vk::PolygonMode::eFill)
                .setFrontFace(vk::FrontFace::eClockwise)
                .setDepthBiasEnable(libraryCI.depthBiasEnable.value_or(vk::False))
                .setDepthBiasConstantFactor(libraryCI.depthBiasConstantFactor.value_or(vk::False))
                .setDepthBiasClamp({})
                .setDepthBiasSlopeFactor(libraryCI.depthBiasSlopeFactor.value_or(vk::False))
                .setLineWidth(1.f);

        auto dy = empty(libraryCI.dynamicStates) ? std::initializer_list<vk::DynamicState>{vk::DynamicState::eViewport, vk::DynamicState::eScissor} : libraryCI.dynamicStates;
        auto dynamicStateCI = vk::PipelineDynamicStateCreateInfo().setDynamicStates(dy);

        auto viewCI = vk::PipelineViewportStateCreateInfo();
        if (empty(libraryCI.viewports)){ viewCI.setViewportCount(1); } else { viewCI.setViewports(libraryCI.viewports); }
        if (empty(libraryCI.rect2d)){ viewCI.setScissorCount(1); } else { viewCI.setScissors(libraryCI.rect2d); }

        vot::vector<vk::PipelineShaderStageCreateInfo> shaderStageCIs;
        //auto shaderStageCI = addShader(libraryCI.shaderPt, vk::ShaderStageFlagBits::eVertex);
        if (!libraryCI.shaderPt.empty()) {
            shaderStageCIs.emplace_back(addShader(libraryCI.shaderPt, vk::ShaderStageFlagBits::eVertex));
            yic::shaderHot->rego(libraryCI.shaderPt, {.gp = this, .flags = vk::ShaderStageFlagBits::eVertex});
        }
        if (!libraryCI.geomShaderPt.empty()) {
            shaderStageCIs.emplace_back(addShader(libraryCI.geomShaderPt, vk::ShaderStageFlagBits::eGeometry));
            yic::shaderHot->rego(libraryCI.geomShaderPt, {.gp = this, .flags = vk::ShaderStageFlagBits::eGeometry});
        }


        auto ci = vk::GraphicsPipelineCreateInfo()
                .setFlags(vk::PipelineCreateFlagBits::eLibraryKHR |
                          vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT)
                .setStages(shaderStageCIs)
          //      .setStages(shaderStageCI)
                .setLayout(mPipelineLibrary.pipelineLayout)
                .setPDynamicState(&dynamicStateCI)
                .setPViewportState(&viewCI)
                .setPRasterizationState(&rasterizationState)
                .setRenderPass(mPipelineLibrary.renderPass)
                .setPNext(&libraryInfo);

        mPipelineLibrary.preRasterizationShaders = ct.device->createGraphicsPipeline(mPipelineCache, ci, nullptr).value;
    }

    auto GraphicsPipeline::buildFragmentOutputInterfaceLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.fragmentOutputInterfaceCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentOutputInterface);

        auto colorBlendAttach = empty(libraryCI.colorBlendAttachmentStates) ? std::initializer_list<vk::PipelineColorBlendAttachmentState>{makePipelineColorBlendAttachments()} : libraryCI.colorBlendAttachmentStates;
        auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
                .setAttachments(colorBlendAttach)
                .setLogicOpEnable({});
        auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

//        auto renderingInfo = vk::PipelineRenderingCreateInfo()
//                .setColorAttachmentFormats(yic::systemHub.val<ev::pVkRenderContext>().surfaceFormat->format);
//        if (pipelineLibrary.renderPass2CI.useRenderingDepth)
//            renderingInfo.setDepthAttachmentFormat(vk::Format::eD32SfloatS8Uint).setStencilAttachmentFormat(vk::Format::eD32SfloatS8Uint);
//
//
//        auto info = pipelineLibrary.renderPass2CI.pipelineRenderingCreateInfo.value_or(renderingInfo);
        if (mPipelineLibrary.renderPass2CI.colorAttachmentFormats_dynamicRenderingEx.empty())
            mPipelineLibrary.renderPass2CI.setColorAttachmentFormats(yic::systemHub.val<ev::pVkRenderContext>().surfaceFormat->format);
        auto info = mPipelineLibrary.renderPass2CI.getPipelineRenderingCreateInfo();

       // auto info = pipelineLibrary.renderPass2CI.pipelineRenderingCI.value_or(renderingInfo);

        if (!mPipelineLibrary.renderPass)
            libraryInfo.setPNext(&info);

        auto ci = vk::GraphicsPipelineCreateInfo()
                .setFlags(vk::PipelineCreateFlagBits::eLibraryKHR |
                          vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT)
                .setLayout(mPipelineLibrary.pipelineLayout)
                .setRenderPass(mPipelineLibrary.renderPass)
                .setPColorBlendState(&colorBlendState)
                .setPMultisampleState(&multisampleState)
                .setPNext(&libraryInfo);

        mPipelineLibrary.fragmentOutputInterface = ct.device->createGraphicsPipeline(mPipelineCache, ci, nullptr).value;
    }

    auto GraphicsPipeline::buildFragmentShaderLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.fragmentShaderCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentShader);

        vot::vector<vk::PipelineShaderStageCreateInfo> shaderStageCIs;
        if (!libraryCI.shaderPt.empty()){
//            auto shaderStageCI = addShader(libraryCI.shaderPt, vk::ShaderStageFlagBits::eFragment);
            shaderStageCIs.emplace_back(addShader(libraryCI.shaderPt, vk::ShaderStageFlagBits::eFragment));
            yic::shaderHot->rego(libraryCI.shaderPt, {.gp = this, .flags = vk::ShaderStageFlagBits::eFragment});
        }

        auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo()
                .setDepthTestEnable(libraryCI.depthTestEnable.value_or(vk::True))
                .setDepthWriteEnable(libraryCI.depthWriteEnable.value_or(vk::True))
                .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
                .setDepthBoundsTestEnable({})
                .setStencilTestEnable({})
                .setFront(vk::StencilOpState{})
                .setBack(vk::StencilOpState{})
                .setMinDepthBounds({})
                .setMaxDepthBounds({});
        auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

        auto ci = vk::GraphicsPipelineCreateInfo()
                .setFlags(vk::PipelineCreateFlagBits::eLibraryKHR |
                          vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT)
                .setStages(shaderStageCIs)
                .setPDepthStencilState(&depthStencilState)
                .setPMultisampleState(&multisampleState)
                .setRenderPass(mPipelineLibrary.renderPass)
                .setLayout(mPipelineLibrary.pipelineLayout)
                .setPNext(&libraryInfo);

        mPipelineLibrary.fragmentShader = ct.device->createGraphicsPipeline(mPipelineCache, ci, nullptr).value;
    }

    auto GraphicsPipeline::buildPipelineLayout(vot::PipelineLibrary &pipelineLibrary) const -> void {
        auto& setLayoutCI = pipelineLibrary.pipelineDescriptorSetLayoutCI;

//        if (setLayoutCI.desSetLayouts.empty() && !setLayoutCI.desSetBindings.empty()) {
//            for (auto &bds: setLayoutCI.desSetBindings) {
//                vk::DescriptorSetLayoutCreateInfo createInfo{{}, bds.second};
//
//                setLayoutCI.desSetLayouts.emplace_back( vot::create("create descriptor set layout") = [&] {
//                    return ct.device->createDescriptorSetLayout(createInfo);
//                });
//            }
//        }
//
//        vk::PipelineLayoutCreateInfo createInfo{ {}, setLayoutCI.desSetLayouts, setLayoutCI.pushConstantRange };

//        pipelineLibrary.pipelineLayout = vot::create("create pipeline layout") = [&]{
//            return ct.device->createPipelineLayout(createInfo);
//        };
//        pipelineLibrary.pipelineLayout = setLayoutCI.buildPipelineSetLayout(ct.device);
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>)
                pipelineLibrary.pipelineLayout = arg.buildPipelineSetLayout(ct.device);
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>)
                pipelineLibrary.pipelineLayout = arg.buildPipelineSetLayout(ct.device);
        }, setLayoutCI);
    }

    auto GraphicsPipeline::buildRenderPass(vot::PipelineLibrary &pipelineLibrary) const -> void {
        auto& rp2Ci = pipelineLibrary.renderPass2CI;

        if (rp2Ci.attachmentDescription2s.empty())
            return;

        auto ci = vk::RenderPassCreateInfo2()
                .setAttachments(rp2Ci.attachmentDescription2s)
                .setDependencies(rp2Ci.subpassDependency2s)
                .setSubpasses(rp2Ci.subpassDescription2s)
                .setCorrelatedViewMasks({});

        pipelineLibrary.renderPass = vot::create("create render pass 2") = [&]{
            return ct.device->createRenderPass2(ci);
        };
    }

    auto GraphicsPipeline::addShader(vot::string pt, vk::ShaderStageFlagBits flags) -> vk::PipelineShaderStageCreateInfo {
        pt = spv_path + pt + ".spv";
        vot::vector<char> v;
        std::ranges::copy(fo::loadFile(pt), std::back_inserter(v));

        auto sm = ct.device->createShaderModuleUnique(vk::ShaderModuleCreateInfo()
                                                              .setCodeSize(sizeof(char) * v.size())
                                                              .setPCode(reinterpret_cast<const uint32_t*>(v.data())));

        mShaderModules.emplace_back(std::move(sm));

        return {{}, flags, mShaderModules.back().get(), "main", {}};
    }

} // rhi

//    auto vot::CommandBuffer::vot::bindPipeline(auto pipeline) -> void {
//
//    }


