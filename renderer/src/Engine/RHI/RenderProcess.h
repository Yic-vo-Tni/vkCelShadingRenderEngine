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

    class RenderProcess {
        using recCommandFn = std::function<void(vk::CommandBuffer& cmd)>;
    public:
        explicit RenderProcess(std::string id);
        ~RenderProcess();

        auto prepare() -> void;
        auto process() -> std::optional<vk::CommandBuffer>;

        auto appendRenderPassProcessSecondaryCommand(const uint8_t &seq, const std::shared_ptr<vkImage> &imgSptr,
                                                     const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessSecondaryCommand(const uint8_t &seq, const vkImage* imgPtr,
                                                     const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessCommand(const uint8_t &seq, const std::vector<vk::Framebuffer> &framebuffers,
                                           const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendProcessCommand(const uint8_t &seq, const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendFinalProcessCommand(const uint8_t &seq) -> void;
        auto updateDescriptor(const uint8_t &seq, const std::shared_ptr<vkImage>& image) -> void;
        auto rebuild() -> void;

        auto& acquire() { return mRenderGroupGraphics; };
        auto& descriptor() { return mDescriptor; };
    private:
        ev::pVkSetupContext ct;
        et::vkRenderContext rt;

        std::string mId;
        uint32_t mImageCount{0};
        std::shared_ptr<vkImage> mOffImage;

        vk::Extent2D mExtent{2560, 1440};
        std::unique_ptr<RenderSession> mRenderSession;

        vot::vector<recCommandFn > mCommandBufRecs;
        std::shared_ptr<RenderGroupGraphics> mRenderGroupGraphics;
        std::shared_ptr<Descriptor> mDescriptor;
        vot::vector<std::shared_ptr<vkImage>> mDyRenderTargetStageImgs;
    };

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPORCESST_H
