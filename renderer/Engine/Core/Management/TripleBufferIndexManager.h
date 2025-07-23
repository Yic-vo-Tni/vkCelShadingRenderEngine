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
            uint8_t oldestIdx = 255;
            uint64_t oldestFrame = UINT64_MAX;
            for (uint8_t i = 0; i < MultiBuffer; ++i) {
                if (!slots[i].ready.load(std::memory_order_seq_cst)) {
                    logicIndex = i;
                    return i;
                }

                auto fid = slots[i].frameId.load(std::memory_order_seq_cst);
                if (fid < oldestFrame) {
                    oldestFrame = fid;
                    oldestIdx = i;
                }
            }

            logicIndex = oldestIdx;
            return oldestIdx;
        }

        auto write_end() -> void {
            ++logicFrameId;
            slots[logicIndex].frameId.store(logicFrameId, std::memory_order_seq_cst);
            slots[logicIndex].ready.store(true, std::memory_order_seq_cst);
        }

        auto read_begin() -> uint8_t {
            uint64_t newestFrame = 0;
            int newestIdx = -1;
            for (uint8_t i = 0; i < MultiBuffer; ++i) {
                if (slots[i].ready.load(std::memory_order_seq_cst)) {
                    auto fid = slots[i].frameId.load(std::memory_order_seq_cst);
                    if (fid > newestFrame) {
                        newestFrame = fid;
                        newestIdx = i;
                    }
                }
            }
            if (newestIdx >= 0) {
                renderIndex = newestIdx;
                return renderIndex;
            }

            return renderIndex;
        }

        auto read_end() -> void{
            slots[renderIndex].ready.store(false, std::memory_order_seq_cst);
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
