//
// Created by lenovo on 7/14/2024.
//

#ifndef VKCELSHADINGRENDERER_RENDERPROCESS_H
#define VKCELSHADINGRENDERER_RENDERPROCESS_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/RenderSession.h"
#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/Allocator.h"
#include "Engine/RHI/FrameRender.h"

namespace yic {

    class RenderProcess {
        using fn_cmdRecord = std::function<void(vk::CommandBuffer& cmd)>;
    public:
        explicit RenderProcess(std::string id);
        ~RenderProcess();

        auto prepare() -> void;
        auto appendCommandRecord(const fn_cmdRecord& record) -> void;
        auto process() -> std::optional<vk::CommandBuffer>;

    private:
        et::vkSetupContext ct;
        et::vkRenderContext rt;

        std::string mId;
        vkImg_sptr mOffImage;
        std::atomic<vk::Extent2D> mExtent;
        std::unique_ptr<RenderSession> mRenderSession;
        std::vector<fn_cmdRecord> mCommandBufferRecords;
    };



} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESS_H
