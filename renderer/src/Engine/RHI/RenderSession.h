//
// Created by lenovo on 7/31/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERSESSION_H
#define VKCELSHADINGRENDERER_RENDERSESSION_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic {

    class RenderSession {
    public:
        struct ClearValue {
            static std::vector<vk::ClearValue> Color() {
                return {vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 0.f}}};
            }

            static std::vector<vk::ClearValue> ColorDepth() {
                return {
                        vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 0.f}},
                        vk::ClearDepthStencilValue{1.f, 0}
                };
            }
        };
        struct RenderPassInfo{
            vk::RenderPass renderPass;
            std::vector<vk::Framebuffer> framebuffers;
            opt<vk::Extent2D> extent{std::nullopt};
            vk::Offset2D offset2D{0, 0};
            std::vector<vk::ClearValue> clearValues;
            vk::SubpassContents subpassContents{vk::SubpassContents::eInline};

            RenderPassInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers,
                           vk::Extent2D extent, const std::vector<vk::ClearValue> &clearValues = ClearValue::Color())
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      extent(extent),
                      clearValues(clearValues) {

            };
            RenderPassInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers,
                           const std::vector<vk::ClearValue> &clearValues = ClearValue::Color())
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      clearValues(clearValues) {

            };
        };
    public:
        RenderSession(std::string  id, const uint32_t& qIndex, const uint32_t& CommandBufferCount);
        ~RenderSession();

        auto beginCommandBuf(vk::Extent2D extent2D) -> vk::CommandBuffer&;
        auto endCommandBuf() -> void;

        auto beginRenderPass(RenderPassInfo passInfo) -> void;
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
        vk::Extent2D mExtent;
        std::vector<vk::CommandBuffer> mCommandBuffers{};
    };


} // yic

#endif //VKCELSHADINGRENDERER_RENDERSESSION_H
