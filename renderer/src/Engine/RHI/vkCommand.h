//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMAND_H
#define VKCELSHADINGRENDERER_VKCOMMAND_H

#include "vkSwapchain.h"

namespace yic {

    class vkCommand {
    public:
        vkCommand(const std::string& id, const uint32_t& qIndex);
        ~vkCommand();

        auto beginCommandBuf() -> void;
        auto endCommandBuf() -> void;

    private:
        auto createCommandPool() -> vk::CommandPool;
        auto createCommandBuffers() -> std::vector<vk::CommandBuffer>;

    private:
        std::string mId{};
        vk::Device mDevice{};
        vk::RenderPass mRenderPass{};
        std::vector<vk::Framebuffer> mFrameBuffers{};
        uint32_t mQueueIndex{UINT32_MAX};
        vk::CommandBuffer mActiveCommandBuffer{};

        vk::CommandPool mCommandPool{};
        std::vector<vk::CommandBuffer> mCommandBuffers{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKCOMMAND_H
