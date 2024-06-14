//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMAND_H
#define VKCELSHADINGRENDERER_VKCOMMAND_H

#include "vkSwapchain.h"

namespace yic {

    class vkCommand {
    public:
        vkCommand();
        ~vkCommand();

        auto beginCommandBuf() -> vk::CommandBuffer;
        auto endCommandBuf(vk::CommandBuffer& cmd) -> void;

    private:
        auto createCommandPool() -> vk::CommandPool;
        auto createCommandBuffers() -> std::vector<vk::CommandBuffer>;

    private:
        vk::Device mDevice{};
        vk::RenderPass mRenderPass{};
        std::vector<vk::Framebuffer> mFrameBuffers{};

        vk::CommandPool mCommandPool{};
        std::vector<vk::CommandBuffer> mCommandBuffers{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKCOMMAND_H
