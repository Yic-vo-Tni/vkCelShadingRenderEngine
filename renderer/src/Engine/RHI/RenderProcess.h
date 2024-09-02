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
#include "Engine/RHI/Descriptor.h"

#include "Engine/RHI/RenderGroup.h"

namespace yic {

    class RenderProcess {
        enum offImageType{
            eStandard, eRayTraced, eComposite, eCount
        };
        using fn_cmdRecord = std::function<void(vk::CommandBuffer& cmd)>;
    public:
        explicit RenderProcess(std::string id);
        ~RenderProcess();

        auto prepare() -> void;
        auto appendCommandRecord(const fn_cmdRecord& record) -> void;
        auto appendCommandRecordExtra(const fn_cmdRecord& rec) -> void;
        auto process() -> std::optional<vk::CommandBuffer>;


    private:
        et::vkSetupContext ct;
        et::vkRenderContext rt;

        std::string mId;
        uint32_t mImageCount{0};
        vkImg_sptr mOffImage;

        std::atomic<vk::Extent2D> mExtent;
        std::unique_ptr<RenderSession> mRenderSession;

        std::shared_ptr<Descriptor> mPostDescriptor;
        std::shared_ptr<RenderGroupGraphics> mRenderGroupGraphics;

        std::vector<fn_cmdRecord> mCommandBufferRecords;
        std::vector<fn_cmdRecord> mCommandBufferRecordExtras;
    };



} // yic

#endif //VKCELSHADINGRENDERER_RENDERPROCESS_H
