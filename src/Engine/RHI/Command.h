//
// Created by lenovo on 9/25/2024.
//

#ifndef VKCELSHADINGRENDERER_COMMAND_H
#define VKCELSHADINGRENDERER_COMMAND_H

#include "Core/Management/IndexManager.h"

namespace rhi {

    class CommandManager {
    public:
        Make = []{ return Singleton<CommandManager>::make_ptr(); };
        CommandManager();
        ~CommandManager();

        auto draw(const std::function<void(vot::CommandBuffer&)>& record) -> void;
        auto drawOneTimeSubmit(const std::function<void(vot::CommandBuffer&)>& rec) -> void;
        auto acquire() -> vot::CommandBuffer;
        auto release(vot::CommandBuffer& cmd) -> void;

     //   auto acquire(const vot::threadSpecificCmdPool& threadSpecificCmdPool) -> vot::CommandBuffer&;

        auto clear() -> void;



        ////
        auto acquire(const vot::threadSpecificCmdPool& threadSpecificCmdPool) -> vot::CommandBuffer*;
        auto acquire(vot::RHandle& rHandle, const vot::threadSpecificCmdPool& threadSpecificCmdPool) -> vot::RHandle {
            if (rHandle == nullptr){ rHandle = acquire(threadSpecificCmdPool); }
            return rHandle;
        }
        auto bind(vot::CommandBuffer* cmd, const std::function<void(vot::CommandBuffer& cmd)>& fn) -> void;
        auto bind(vot::SubmitInfo submitInfo, const std::function<void(vot::CommandBuffer& cmd)>& fn) -> void;
    private:
        auto alloc(const vot::threadSpecificCmdPool& threadSpecificCmdPool) -> void;
        auto allocCommandBuffer() -> void;
    private:
        ev::pVkSetupContext ct{};
        uint8_t mMaxPoolCount{12};
        std::mutex mMutex;
        std::condition_variable mCommandAvailable;
        vot::vector<vk::CommandPool> mCommandPools;
        vot::vector<vk::Fence> mFences;
        oneapi::tbb::concurrent_queue<vot::CommandBuffer> mAvailablePrimaryCommandbuffers;

        uint32_t* mActiveImageIndex;
        vot::vector<vk::CommandPool> mThreadCommandPools;
        vot::vector<vot::vector<vot::CommandBuffer>> mThreadCommandbuffers;

        ///
        uint32_t mFrameCount;
        yic::IndexManager mIndexManager;
    };

} // rhi

namespace yic{
    inline rhi::CommandManager* command;
}

#endif //VKCELSHADINGRENDERER_COMMAND_H
