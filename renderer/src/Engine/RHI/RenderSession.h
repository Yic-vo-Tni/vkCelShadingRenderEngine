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
        struct clearValue{
            static inline vot::vector<vk::ClearValue> color{vk::ClearColorValue{0.19f, 0.2f, 0.25f, 1.0f}};
            static inline vot::vector<vk::ClearValue> colorDepth{vk::ClearColorValue{0.19f, 0.2f, 0.25f, 1.0f}, vk::ClearDepthStencilValue{1.f, 0}};
        };
        struct passInfo{
            vk::RenderPass renderPass;
            const std::vector<vk::Framebuffer>& framebuffers;
            opt<vk::Extent2D> extent{std::nullopt};
            vk::Offset2D offset2D{0, 0};
            const vot::vector<vk::ClearValue>& clearValues;
            vk::SubpassContents subpassContents{vk::SubpassContents::eInline};

            passInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers,
                           vk::Extent2D extent, const vot::vector<vk::ClearValue> &clearValues = clearValue::color)
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      extent(extent),
                      clearValues(clearValues) {

            };
            passInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers,
                           const vot::vector<vk::ClearValue> &clearValues = clearValue::color)
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      clearValues(clearValues) {

            };
            passInfo(vk::RenderPass RenderPass, const std::vector<vk::Framebuffer> &framebuffers, vk::SubpassContents subpassContents,
                           const vot::vector<vk::ClearValue> &clearValues = clearValue::color)
                    : renderPass(RenderPass),
                      framebuffers(framebuffers),
                      subpassContents(subpassContents),
                      clearValues(clearValues) {

            };
        };
    public:
        RenderSession(std::string  id, const uint32_t& qIndex, const uint32_t& CommandBufferCount);
        RenderSession(std::string  id, const uint32_t& qIndex, const uint32_t& CommandBufferCount, vk::CommandBufferUsageFlags flags);
        ~RenderSession();

        auto beginCommandBuf(vk::Extent2D extent2D) -> vk::CommandBuffer&;
        auto endCommandBuf() -> void;

        auto beginRenderPass(passInfo info) -> void;
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
        vk::CommandBufferUsageFlags mFlags{};
    };


} // yic

#endif //VKCELSHADINGRENDERER_RENDERSESSION_H
