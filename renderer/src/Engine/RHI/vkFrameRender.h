//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKFRAMERENDER_H
#define VKCELSHADINGRENDERER_VKFRAMERENDER_H

#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkFrameRender {
    public:
        vkFrameRender();
        ~vkFrameRender();

        auto createImguiFrameRenderPass() -> void;
    private:
        auto createRenderPass(const std::vector<vk::AttachmentDescription>& attachDes,
                              const std::vector<vk::SubpassDescription>& subpass,
                              const std::vector<vk::SubpassDependency>& dependency,
                              const std::string& des = {}) -> vk::RenderPass;
        auto createFrameBuffer(vk::RenderPass renderPass, vk::Extent2D extent, const std::vector<std::vector<vk::ImageView>>& attach) -> std::vector<vk::Framebuffer>;

    private:
        vk::Device mDevice{};

    };

} // yic

#endif //VKCELSHADINGRENDERER_VKFRAMERENDER_H
