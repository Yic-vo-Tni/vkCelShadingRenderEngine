#include <utility>

//
// Created by lenovo on 9/26/2024.
//

#ifndef VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
#define VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H

namespace rhi {

    class PipeRSManager {
    public:
        Make = []{ return Singleton<PipeRSManager>::make_ptr(); };
        PipeRSManager();
        ~PipeRSManager();

        auto gPipeHandle(vk::PipelineCache cache, const vk::GraphicsPipelineCreateInfo& ci) -> vk::Pipeline {
            auto pipeline = ct.device->createGraphicsPipeline(cache, ci, nullptr).value;
            pipelines.emplace_back(pipeline);
            return pipeline;
        };
        auto gPipeLayoutHandle(const vk::PipelineLayout& layout) -> vk::PipelineLayout {
            pipelineLayouts.emplace_back(layout);
            return layout;
        }

        auto clear() -> void {
            for (auto& pipe : pipelines) {
                if (pipe && pipe != VK_NULL_HANDLE) {
                    ct.device->destroy(pipe);
                    pipe = VK_NULL_HANDLE;
                }
            }
            for (auto& layout : pipelineLayouts) {
                if (layout && layout != VK_NULL_HANDLE) {
                    ct.device->destroy(layout);
                    layout = VK_NULL_HANDLE;
                }
            }
        };
    private:
        ev::pVkSetupContext ct{};
        vot::vector<vk::Pipeline> pipelines;
        vot::vector<vk::PipelineLayout> pipelineLayouts;
    };
    inline PipeRSManager* PipeRSManager;

    class GraphicsPipeline final : public vot::IPipeline{
    public:
        vot::DescriptorHandle DS;
    public:
        GraphicsPipeline();
        ~GraphicsPipeline() override;

        auto combinePipelineLibrary(vot::PipelineLibrary pipelineLibrary) -> void;

        static vk::PipelineColorBlendAttachmentState makeColorBlendAttachment(
            const vot::gfx::api::PipelineColorBlendAttachmentStateCI &state = {}) {
            //format-off
            return vk::PipelineColorBlendAttachmentState(
                state.blendEnable,
                state.srcColorBF, state.dstColorBF, state.colorBlendOp,
                state.srcAlphaBF, state.dstAlphaBF, state.alphaBlendOp,
                state.colorFlags);
            //format-on
        }
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
