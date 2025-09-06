//
// Created by lenovo on 9/26/2024.
//

#include "GraphicsPipeline.h"
#include "Core/DispatchSystem/SystemHub.h"
#include "Utils/FileOperation.h"

#include "Editor/ShaderHotReload/ShaderHotReload.h"

namespace rhi {
    PipeRSManager::PipeRSManager() : ct(yic::systemHub.va<ev::pVkSetupContext>()) {}
    PipeRSManager::~PipeRSManager() = default;

    GraphicsPipeline::GraphicsPipeline() : ct(yic::systemHub.va<ev::pVkSetupContext>()) { }

    GraphicsPipeline::~GraphicsPipeline() {
        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>) arg.clear();
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>) arg.clear(ct.device);
        }, mPipelineLibrary.pipelineDescriptorSetLayoutCI);

        auto de = [&](auto& pipe){
            if (pipe && pipe != VK_NULL_HANDLE){
                ct.device->destroy(pipe);
                pipe = VK_NULL_HANDLE;
            }
        };

        de(mPipelineLibrary.renderPass);
        de(mFinalPipeline);
    }

    auto GraphicsPipeline::combinePipelineLibrary(vot::PipelineLibrary pipelineLibrary) -> void {
        if (!pipelineLibrary.pipelineLayout) buildPipelineLayout(pipelineLibrary);
        if (!pipelineLibrary.renderPass) buildRenderPass(pipelineLibrary);

        mPipelineLibrary = std::move(pipelineLibrary);

        if (!mPipelineLibrary.vertexInputInterface) buildVertexInputInterfaceLibrary();
        if (!mPipelineLibrary.preRasterizationShaders) buildPreRasterizationShadersLibrary();
        if (!mPipelineLibrary.fragmentOutputInterface) buildFragmentOutputInterfaceLibrary();
        if (!mPipelineLibrary.fragmentShader) buildFragmentShaderLibrary();

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

        if (mFinalPipeline){
            ct.device->destroy(mFinalPipeline);
            mFinalPipeline = VK_NULL_HANDLE;
        }
        mFinalPipeline = vot::create("create pipeline") = [&]{
            return ct.device->createGraphicsPipeline(mPipelineCache, vk::GraphicsPipelineCreateInfo()
                    .setPNext(&libraryCI)
                    .setLayout(mPipelineLibrary.pipelineLayout)).value;
        };
    }

    auto GraphicsPipeline::buildVertexInputInterfaceLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.vertexInputInterfaceCI;
        constexpr auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eVertexInputInterface);

        const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{{},
                                                                    libraryCI.primitiveTopology.value_or(vk::PrimitiveTopology::eTriangleList),
                                                                    {}};

        const auto vertexInputState = vk::PipelineVertexInputStateCreateInfo()
                .setVertexBindingDescriptions(libraryCI.vertexInputBindings)
                .setVertexAttributeDescriptions(libraryCI.vertexInputAttributes);

        const auto ci = vk::GraphicsPipelineCreateInfo()
                .setFlags(vk::PipelineCreateFlagBits::eLibraryKHR |
                          vk::PipelineCreateFlagBits::eRetainLinkTimeOptimizationInfoEXT)
                .setPInputAssemblyState(&inputAssemblyState)
                .setPVertexInputState(&vertexInputState)
                .setPNext(&libraryInfo);

        mPipelineLibrary.vertexInputInterface = PipeRSManager->gPipeHandle(mPipelineCache, ci);
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
                .setLayout(mPipelineLibrary.pipelineLayout)
                .setPDynamicState(&dynamicStateCI)
                .setPViewportState(&viewCI)
                .setPRasterizationState(&rasterizationState)
                .setRenderPass(mPipelineLibrary.renderPass)
                .setPNext(&libraryInfo);

        mPipelineLibrary.preRasterizationShaders = PipeRSManager->gPipeHandle(mPipelineCache, ci);
    }

    auto GraphicsPipeline::buildFragmentOutputInterfaceLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.fragmentOutputInterfaceCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentOutputInterface);

        auto colorBlendAttach = libraryCI.colorBlendAttachmentStates.empty() ? std::initializer_list<vk::PipelineColorBlendAttachmentState>{makePipelineColorBlendAttachments()} : libraryCI.colorBlendAttachmentStates;
        auto colorBlendState = vk::PipelineColorBlendStateCreateInfo()
                .setAttachments(colorBlendAttach)
                .setLogicOpEnable({});
        auto multisampleState = vk::PipelineMultisampleStateCreateInfo()
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);

        if (mPipelineLibrary.renderPass2CI.colorAttachmentFormats_dynamicRenderingEx.empty())
            mPipelineLibrary.renderPass2CI.setColorAttachmentFormats({yic::systemHub.va<ev::pVkRenderContext>().surfaceFormat->format});
        auto info = mPipelineLibrary.renderPass2CI.getPipelineRenderingCreateInfo();

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

        mPipelineLibrary.fragmentOutputInterface = PipeRSManager->gPipeHandle(mPipelineCache, ci);
    }

    auto GraphicsPipeline::buildFragmentShaderLibrary() -> void {
        auto& libraryCI = mPipelineLibrary.fragmentShaderCI;
        auto libraryInfo = vk::GraphicsPipelineLibraryCreateInfoEXT()
                .setFlags(vk::GraphicsPipelineLibraryFlagBitsEXT::eFragmentShader);

        vot::vector<vk::PipelineShaderStageCreateInfo> shaderStageCIs;

        if (!libraryCI.shaderPt.empty()){
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

        mPipelineLibrary.fragmentShader = PipeRSManager->gPipeHandle(mPipelineCache, ci);
    }

    auto GraphicsPipeline::buildPipelineLayout(vot::PipelineLibrary &pipelineLibrary) const -> void {
        auto& setLayoutCI = pipelineLibrary.pipelineDescriptorSetLayoutCI;

        std::visit([&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI>)
                pipelineLibrary.pipelineLayout = PipeRSManager->gPipeLayoutHandle(arg.buildPipelineSetLayout(ct.device));
            if constexpr (std::is_same_v<T, vot::PipelineDescriptorSetLayoutCI2>)
                pipelineLibrary.pipelineLayout = PipeRSManager->gPipeLayoutHandle(arg.buildPipelineSetLayout(ct.device));
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




