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

//        auto prepare() -> void;
        auto process() -> std::optional<vk::CommandBuffer>;

        auto appendRenderPassProcessSecondaryCommand(const uint8_t &seq, const std::shared_ptr<vkImage> &imgSptr,
                                                     const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessSecondaryCommand(const uint8_t &seq, const vkImage* imgPtr,
                                                     const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessCommand(const uint8_t &seq, const std::vector<vk::Framebuffer> &framebuffers,
                                           const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessCommand(const uint8_t &seq, const vkImage_sptr& imageSptr,
                                            const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendRenderPassProcessCommandPrepose(const yic2::Image_sptr &imageSptr,
                                            const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendProcessCommand(const uint8_t &seq, const yic::RenderProcess::recCommandFn &rec) -> void;
        auto appendFinalProcessCommand(const uint8_t &seq) -> void;
      //  auto updateDescriptor(const uint8_t &seq, const std::shared_ptr<vkImage>& image) -> void;

        template<typename T>
        auto updateDescriptor(T e, const std::variant<vkImage_sptr, yic2::Image_sptr>& image) -> void{
            auto seq = static_cast<uint8_t>(e);
            updateDescriptorImpl(seq, image);
        }
        auto rebuild() -> void;

        auto& acquire() { return mRenderGroupGraphics; };
        auto& descriptor() { return mDescriptor; };

    private:
        auto updateDescriptorImpl(const uint8_t &seq, const std::variant<vkImage_sptr, yic2::Image_sptr>& image) -> void;
        auto renderTargetChange() -> void;
    private:
        ev::pVkSetupContext ct{};
        ev::hVkRenderContext rt{};

        std::string mId;
        uint32_t mImageCount{0};
        uint32_t* mImageIndex{};
        std::shared_ptr<vkImage> mOffImage;

        vk::Extent2D mExtent{2560, 1440};
        std::unique_ptr<RenderSession> mRenderSession;

        vot::vector<recCommandFn> mCommandBufRecsPrepose;
        vot::vector<recCommandFn> mCommandBufRecs;
        std::shared_ptr<RenderGroupGraphics> mRenderGroupGraphics;
        std::shared_ptr<Descriptor> mDescriptor;
        vot::vector<std::shared_ptr<vkImage>> mDyRenderTargetStageImgs;

        vot::vector<std::variant<vkImage_sptr, yic2::Image_sptr>> mDynamicRenderTargetStageImages;
    };

    using RenderProcessHanlde = RenderProcess*;

} // yic

#endif //VKCELSHADINGRENDERER_RENDERPORCESST_H
