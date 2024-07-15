//
// Created by lenovo on 7/14/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRENDER_H
#define VKCELSHADINGRENDERER_VKRENDER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/vkCommand.h"
#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/vkAllocator.h"

namespace yic {

    class vkRender {
    public:
        vkRender();
        ~vkRender();

        auto render() -> vk::CommandBuffer;

        auto createRenderPass() -> vk::RenderPass;
        auto createFramebuffers(const vkImg_sptr& imgSptr, vk::RenderPass renderPass) -> std::vector<vk::Framebuffer>;
    private:
        et::vkSetupContext ct;
        et::vkRenderContext rt;

        vk::Extent2D mExtent{1920, 1080};
        vk::RenderPass mRenderPass;
        std::vector<vk::Framebuffer> mFramebuffers;

        std::string mId_Primary{"Primary"};
        vkImg_sptr mOffImage_Primary;
        std::unique_ptr<vkCommand> mCommand_Primary;
        std::shared_ptr<vkDescriptor> mDescriptor_Primary;
        std::unique_ptr<GraphicsPipeline> mPipeline_Primary;
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKRENDER_H
