//
// Created by lenovo on 8/31/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPORCESST_H
#define VKCELSHADINGRENDERER_RENDERPORCESST_H


#include "Engine/Utils/Log.h"

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/Descriptor.h"
#include "Engine/RHI/RenderSession.h"
#include "Engine/RHI/RenderGroup.h"

namespace yic {

    class RenderProcessT {
        using recCommandFn = std::function<void(vk::CommandBuffer& cmd)>;
    public:
        explicit RenderProcessT(std::string id);
        ~RenderProcessT();

        auto prepare() -> void;
        auto process() -> std::optional<vk::CommandBuffer>;

        auto appendRenderPassProcessSecondaryCommand(const uint8_t &seq, const std::vector<vk::Framebuffer> &framebuffers,
                                                const yic::RenderProcessT::recCommandFn &rec) -> void;
        auto appendRenderPassProcessCommand(const uint8_t &seq, const std::vector<vk::Framebuffer> &framebuffers,
                                           const yic::RenderProcessT::recCommandFn &rec) -> void;
        auto appendProcessCommand(const uint8_t &seq, const yic::RenderProcessT::recCommandFn &rec) -> void;
        auto appendFinalProcessCommand(const yic::RenderProcessT::recCommandFn &rec) -> void;
        auto updateDescriptor(const uint8_t &seq, const std::shared_ptr<vkImage>& image) -> void;

        auto& acquire() { return mRenderGroupGraphics; };
        auto& descriptor() { return mDescriptor; };
    private:
        et::vkSetupContext ct;
        et::vkRenderContext rt;

        std::string mId;
        uint32_t mImageCount{0};
        std::shared_ptr<vkImage> mOffImage;

        std::atomic<vk::Extent2D> mExtent;
        std::unique_ptr<RenderSession> mRenderSession;

        //oneapi::tbb::concurrent_map<uint8_t, recCommandFn> mCommandBufRecs;
        std::map<uint8_t, recCommandFn> mCommandBufRecs;
        RenderGroupGraphics_sptr mRenderGroupGraphics;
        std::shared_ptr<Descriptor> mDescriptor;

        // ttt
        std::map<uint8_t, std::shared_ptr<vkImage>> mDynamicRenderTargetStageImages;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPORCESST_H
