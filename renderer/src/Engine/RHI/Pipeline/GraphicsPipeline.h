//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
#define VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H

#include "State.h"

namespace yic {

    struct Graphics : public State<Graphics>{
    private:
        struct ShaderModule{
            vk::ShaderModule shaderModule{};
            vk::ShaderStageFlagBits flags{};
        };
    public:
        Graphics(vk::Device device, vk::PipelineLayout layout, vk::RenderPass renderPass);
        ~Graphics(){
            for(auto& m : shaderModules){
                mDevice.destroy(m.second.shaderModule);
            }
            mDevice.destroy(mPipeline);
        }

        Graphics& create(){
            updateState();

            createPipeline();
            return *this;
        };

        static inline vk::PipelineColorBlendAttachmentState makePipelineColorBlendAttachments(
                vk::ColorComponentFlags colorFlags = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                                                     | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA,
                vk::Bool32 blendEnable = true,
                vk::BlendFactor srcColorBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstColorBF = vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp colorBlendOp = vk::BlendOp::eAdd,
                vk::BlendFactor srcAlphaBF = vk::BlendFactor::eSrcAlpha, vk::BlendFactor dstAlphaBF = vk::BlendFactor::eOneMinusSrcAlpha,
                vk::BlendOp alphaBlendOp = vk::BlendOp::eAdd){
            vk::PipelineColorBlendAttachmentState att{blendEnable,
                                                      srcColorBF, dstColorBF, colorBlendOp,
                                                      srcAlphaBF, dstAlphaBF, alphaBlendOp,
                                                      colorFlags};
            return att;
        };
        void updateState();

        Graphics& addBindingDescription(const vk::VertexInputBindingDescription& bindDes){
            bindingDescriptions.emplace_back(bindDes);
            return *this;
        }
        Graphics& addAttributeDescription(const vk::VertexInputAttributeDescription& attrDes){
            attributeDescriptions.emplace_back(attrDes);
            return *this;
        }

        Graphics& addShader(const std::string& path, vk::ShaderStageFlagBits flags){
            auto t = [this, path, flags]{
                std::vector<char> v;
                std::ranges::copy(fo::loadFile(path), std::back_inserter(v));

                if (shaderModules[path].shaderModule)
                    mDevice.destroy(shaderModules[path].shaderModule);

                shaderModules[path].shaderModule = mDevice.createShaderModule(vk::ShaderModuleCreateInfo().setCodeSize(sizeof (char) * v.size())
                                                                                      .setPCode(reinterpret_cast<const uint32_t*>(v.data())));
                shaderModules[path].flags = flags;
            };

            TaskBus::registerShaderFileTask(path, [this, t]{
                t();
                createPipeline();
            });

            t();

            return *this;
        }

        auto createShaderStage() -> void{
            shaderStages.clear();
            for(auto& [p, m] : shaderModules){
                vk::PipelineShaderStageCreateInfo shaderStage{{}, m.flags, m.shaderModule, "main"};
                shaderStages.push_back(shaderStage);
            }
        }

        auto createPipeline() -> void{
            if (mPipeline){
                mDevice.waitIdle();
                mDevice.destroy(mPipeline);
            }

            createShaderStage();
            createInfo.setStages(shaderStages);
            mPipeline = vkCreate("create graphics pipeline") = [&]{
                return mDevice.createGraphicsPipeline({}, createInfo).value;
            };
        }

        [[nodiscard]] inline auto& Get() const { return mPipeline;}

    private:
        vk::Device mDevice{};

        vk::Pipeline mPipeline{};
    public:
        vk::GraphicsPipelineCreateInfo createInfo{};
        
        vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        vk::PipelineVertexInputStateCreateInfo vertexInputState;
        vk::PipelineDynamicStateCreateInfo dynamicState;
        vk::PipelineViewportStateCreateInfo viewportState;
        vk::PipelineRasterizationStateCreateInfo rasterizationState;
        vk::PipelineMultisampleStateCreateInfo multisampleState;
        vk::PipelineDepthStencilStateCreateInfo depthStencilState;
        vk::PipelineColorBlendStateCreateInfo colorBlendState;

        std::vector<vk::PipelineColorBlendAttachmentState> blendAttachmentStates{};
    private:
        std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        std::vector<vk::VertexInputBindingDescription> bindingDescriptions{};
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};

        std::vector<vk::Viewport> viewports{};
        std::vector<vk::Rect2D> scissors{};

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{};
        std::unordered_map<std::string, ShaderModule> shaderModules{};
        //std::vector<vk::ShaderModule> temporaryModules{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
