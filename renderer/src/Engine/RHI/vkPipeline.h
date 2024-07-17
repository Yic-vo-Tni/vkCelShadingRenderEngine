//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_VKPIPELINE_H
#define VKCELSHADINGRENDERER_VKPIPELINE_H

#include "Pipeline/GraphicsPipeline.h"
#include "Engine/Core/FileOperator/ShaderHotReloader.h"

namespace yic {

    template<typename state>
    class vkPipeline : public state{
    public:
        template<typename... Args>
        explicit vkPipeline(Args&&...args) : state(std::forward<Args>(args)...){}
        vkPipeline(vk::PipelineLayout layout, vk::RenderPass renderPass) : state(EventBus::Get::vkSetupContext().device_ref(), layout, renderPass){}
        vkPipeline(vk::RenderPass renderPass) : state(EventBus::Get::vkSetupContext().device_ref(), renderPass){}
    };

    using GraphicsPipeline = vkPipeline<Graphics>;

} // yic

#endif //VKCELSHADINGRENDERER_VKPIPELINE_H
