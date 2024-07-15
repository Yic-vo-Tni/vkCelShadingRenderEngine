//
// Created by lenovo on 6/10/2024.
//

#ifndef VKCELSHADINGRENDERER_VKCOMMAND_H
#define VKCELSHADINGRENDERER_VKCOMMAND_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class vkCommand {
    public:
        struct vkClearValueBuilder {
            static std::vector<vk::ClearValue> colorClearValue() {
                return {vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 0.f}}};
            }

            static std::vector<vk::ClearValue> colorDepthClearValue() {
                return {
                        vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 0.f}},
                        vk::ClearDepthStencilValue{1.f, 0}
                };
            }
        };
        struct vkRenderPassInfo{
            vk::RenderPass renderPass;
            std::vector<vk::Framebuffer> framebuffers;
            vk::Extent2D extent;
            vk::Offset2D offset2D{0, 0};
            std::vector<vk::ClearValue> clearValues;
            vk::SubpassContents subpassContents{vk::SubpassContents::eInline};

            vkRenderPassInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers,
                             vk::Extent2D extent, const std::vector<vk::ClearValue> &clearValues)
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      extent(extent),
                      clearValues(clearValues) {

            };
        };
    public:
        vkCommand(std::string  id, const uint32_t& qIndex, const uint32_t& CommandBufferCount);
        ~vkCommand();

        auto beginCommandBuf(vk::Extent2D extent2D) -> void;
        auto endCommandBuf() -> void;

        auto beginRenderPass(vkRenderPassInfo passInfo) -> void;
        auto endRenderPass() -> void;

    private:
        auto createCommandPool() -> vk::CommandPool;
        auto createCommandBuffers(uint32_t cmdCount) -> std::vector<vk::CommandBuffer>;

    private:
        std::string mId{};
        vk::Device mDevice{};
        uint32_t mQueueIndex{UINT32_MAX};
        vk::CommandBuffer mActiveCommandBuffer{};
        vk::CommandPool mCommandPool{};
        std::vector<vk::CommandBuffer> mCommandBuffers{};
    };

} // yic

#endif //VKCELSHADINGRENDERER_VKCOMMAND_H
