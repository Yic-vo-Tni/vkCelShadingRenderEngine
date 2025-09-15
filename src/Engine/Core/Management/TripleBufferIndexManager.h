//
// Created by lenovo on 7/23/2025.
//

#ifndef VKCELSHADINGRENDERER_TRIPLEBUFFERINDEXMANAGER_H
#define VKCELSHADINGRENDERER_TRIPLEBUFFERINDEXMANAGER_H

#include "External/pch.h"
#include "External/base.h"

namespace vot{
    enum LogicBufferType : uint8_t {
        eFast,
        eSlow,
        eCount,
    };
}

namespace hide{

    class TripleBufferIndexManager {
        struct alignas(64) SlotInfo{
            std::atomic<uint64_t> frameId{0};
            std::atomic<bool> ready{false};
        };
        static constexpr uint8_t MultiBuffer{3};
    public:
        auto write_begin() -> uint8_t {
            uint8_t oldestIdx = 0;
            uint64_t oldestFrame = UINT64_MAX;
            for (uint8_t i = 0; i < MultiBuffer; ++i) {
                if (!slots[i].ready.load(std::memory_order_acquire)) {
                    logicIndex = i;
                    return i;
                }
                auto fid = slots[i].frameId.load(std::memory_order_relaxed);
                if (fid < oldestFrame) {
                    oldestFrame = fid;
                    oldestIdx = i;
                }
            }
            logicIndex = oldestIdx;
            return logicIndex;
        }

        auto write_end() -> void {
            ++logicFrameId;
            slots[logicIndex].frameId.store(logicFrameId, std::memory_order_release);
            slots[logicIndex].ready.store(true, std::memory_order_release);
            latestIndex.store(logicIndex, std::memory_order_release);
        }

        auto read_begin() -> uint8_t {
            int idx = latestIndex.load(std::memory_order_acquire);
            if (idx >= 0 && slots[idx].ready.load(std::memory_order_acquire)) {
                renderIndex = idx;
                return renderIndex;
            }
            return 255;
        }

        auto read_end() -> void {
        }

        void print_slots() {
            printf("logicIndex=%u, renderIndex=%u\n", logicIndex, renderIndex);
            for (int i = 0; i < MultiBuffer; ++i) {
                printf("slot[%d]: ready=%d, frameId=%llu\n", i, (int)slots[i].ready.load(), slots[i].frameId.load());
            }
        }

        [[nodiscard]] auto logic_cur() const { return logicIndex; }
        [[nodiscard]] auto render_cur() const { return renderIndex; }
    private:
        SlotInfo slots[MultiBuffer] = {};
        uint8_t logicIndex{}, renderIndex{};
        uint64_t logicFrameId = 0;

        std::atomic_int latestIndex{-1};
    };

    class TripleBufferIndexManagerSet{
    public:
        TripleBufferIndexManager fastBuffer;
        TripleBufferIndexManager slowBuffer;

        TripleBufferIndexManager& get(vot::LogicBufferType type) {
            switch(type) {
                case vot::LogicBufferType::eFast: return fastBuffer;
                case vot::LogicBufferType::eSlow: return slowBuffer;
                default: assert(false); return fastBuffer;
            }
        }
    };

}

namespace yic{
    inline auto& indexRing = Singleton<hide::TripleBufferIndexManagerSet>::make();
}


#endif //VKCELSHADINGRENDERER_TRIPLEBUFFERINDEXMANAGER_H
