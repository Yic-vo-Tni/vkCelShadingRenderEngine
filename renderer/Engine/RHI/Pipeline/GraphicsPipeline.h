#include <utility>

//
// Created by lenovo on 9/26/2024.
//

#ifndef VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
#define VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H

namespace rhi {

    class GraphicsPipeline : public vot::IPipeline{
    public:
        vot::DescriptorHandle DS;
    public:
        GraphicsPipeline();
        ~GraphicsPipeline();

        auto combinePipelineLibrary(vot::PipelineLibrary pipelineLibrary) -> void;
        static auto makePipelineColorBlendAttachments(
                vk::ColorComponentFlags colorFlags = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                     | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                vk::Bool32 blendEnable = true,
                vk::BlendFactor srcColorBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstColorBF = vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
                vk::BlendFactor srcAlphaBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstAlphaBF = vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd) -> vk::PipelineColorBlendAttachmentState{
            vk::PipelineColorBlendAttachmentState att{blendEnable,
                                                      srcColorBF, dstColorBF, colorBlendOp,
                                                      srcAlphaBF, dstAlphaBF, alphaBlendOp,
                                                      colorFlags};
            return att;
        };
//        auto acquirePipelineLibrary() { return mPipelineLibrary; }
        auto acquirePipelineLibrary() { return mPipelineLibrary; }

        vk::Pipeline& acquire() override { return mFinalPipeline; }
        vk::PipelineBindPoint acquirePipelineBindPoint() override { return vk::PipelineBindPoint::eGraphics; }
        auto& acquirePipelineLayout() { return mPipelineLibrary.pipelineLayout; }

        auto buildVertexInputInterfaceLibrary() -> void;
        auto buildPreRasterizationShadersLibrary() -> void;
        auto buildFragmentOutputInterfaceLibrary() -> void;
        auto buildFragmentShaderLibrary() -> void;
        auto build() -> void;
    private:
        auto buildPipelineLayout(vot::PipelineLibrary& pipelineLibrary) const -> void;
        auto buildRenderPass(vot::PipelineLibrary& pipelineLibrary) const -> void;

        [[nodiscard]] auto addShader(vot::string pt, vk::ShaderStageFlagBits flags) -> vk::PipelineShaderStageCreateInfo;
    private:
        ev::pVkSetupContext ct{};

        vot::PipelineLibrary mPipelineLibrary;
        vk::Pipeline mFinalPipeline;
        vk::PipelineCache mPipelineCache;
        vot::vector<vk::UniqueShaderModule> mShaderModules;
    };

} // rhi

#endif //VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
