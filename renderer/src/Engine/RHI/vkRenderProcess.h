//
// Created by lenovo on 7/14/2024.
//

#ifndef VKCELSHADINGRENDERER_VKRENDERPROCESS_H
#define VKCELSHADINGRENDERER_VKRENDERPROCESS_H

#include "Engine/Core/DispatchSystem/Schedulers.h"
#include "Engine/Utils/Log.h"

#include "Engine/RHI/vkAsset.h"
#include "Engine/RHI/vkCommand.h"
#include "Engine/RHI/vkPipeline.h"
#include "Engine/RHI/vkAllocator.h"
#include "Engine/RHI/vkFrameRender.h"

namespace yic {

    class vkRenderProcess {
        using fn_cmdRecord = std::function<void(vk::CommandBuffer& cmd)>;
    public:
        vkRenderProcess(vk::Extent2D extent, std::string id);
        ~vkRenderProcess();

        auto appendCommandRecord(const fn_cmdRecord& record) -> void;
        auto process() -> vk::CommandBuffer;

    private:
        et::vkSetupContext ct;
        et::vkRenderContext rt;

        std::string mId;
        vkImg_sptr mOffImage;
        vk::Extent2D mExtent;
        std::unique_ptr<vkCommand> mCommand;
        std::vector<fn_cmdRecord> mCommandBufferRecords;
    };



} // yic

#endif //VKCELSHADINGRENDERER_VKRENDERPROCESS_H
