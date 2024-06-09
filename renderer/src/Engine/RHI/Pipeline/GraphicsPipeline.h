//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
#define VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H

#include "State.h"

namespace yic {

    struct Graphics : public State<Graphics>{
        Graphics(vk::Device device, vk::PipelineLayout layout, vk::RenderPass renderPass);

        Graphics& create(){
            updateState();
            createInfo.setStages(shaderStages);
            mPipeline = vkCreate("create graphics pipeline") = [&]{
                return mDevice.createGraphicsPipeline({}, createInfo).value;
            };
            return *this;
        };

        Graphics& clearShader(){
            shaderStages.clear();
            temporaryModules.clear();
            return *this;
        }

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

        vk::PipelineShaderStageCreateInfo& addShader(const std::string& code, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            std::vector<char> v;
            std::copy(code.begin(), code.end(), std::back_inserter(v));
            return addShader(v, flags, entryPoint);
        };
        template<typename T>
        vk::PipelineShaderStageCreateInfo& addShader(const std::vector<T>& code, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            vk::ShaderModuleCreateInfo shaderModuleCreateInfo{{}, sizeof(T) * code.size(), reinterpret_cast<const uint32_t*>(code.data())};

            vk::ShaderModule shaderModule{mDevice.createShaderModule(shaderModuleCreateInfo)};
            temporaryModules.push_back(shaderModule);

            return addShader(shaderModule, flags, entryPoint);
        };
        vk::PipelineShaderStageCreateInfo& addShader(vk::ShaderModule shaderModule, vk::ShaderStageFlagBits flags, const char* entryPoint = "main"){
            vk::PipelineShaderStageCreateInfo shaderStage{{}, flags, shaderModule, entryPoint};
            shaderStages.push_back(shaderStage);

            return shaderStages.back();
        };

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
        std::vector<vk::ShaderModule> temporaryModules{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_GRAPHICSPIPELINE_H
