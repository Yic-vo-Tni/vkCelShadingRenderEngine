//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_CommandCollector_H
#define VKCELSHADINGRENDERER_CommandCollector_H

namespace rhi2 {

    class CommandCollector {
        using CommandFn = std::function<void(vot::CommandBuffer&)>;
        constexpr static std::uint32_t numThread = 4;
        constexpr static std::uint32_t numExpandCmd = 32;
        struct SafeCommandPool {
            vk::CommandPool commandPool;
            oneapi::tbb::spin_rw_mutex mutex;
            std::atomic<std::uint32_t> next = 0;
            vot::vector<vot::CommandBuffer> cmds;

            SafeCommandPool() = default;
            ~SafeCommandPool() = default;

            SafeCommandPool(const SafeCommandPool&) = delete;
            SafeCommandPool& operator=(const SafeCommandPool&) = delete;

            SafeCommandPool(SafeCommandPool&& other) noexcept
                : commandPool(other.commandPool),
                  next(other.next.load()),
                  cmds(std::move(other.cmds)) {}

            SafeCommandPool& operator=(SafeCommandPool&& other) noexcept {
                commandPool = std::move(other.commandPool);
                next.store(other.next.load());
                cmds = std::move(other.cmds);
                return *this;
            }
        };
        struct FramePool {
            vot::vector<SafeCommandPool> commandPools;
            oneapi::tbb::concurrent_vector<vot::CommandBuffer> records;
            vk::Fence fence;
            std::atomic<std::uint32_t> id = 0;

            FramePool() = default;

            FramePool(const FramePool&) = delete;
            FramePool& operator=(const FramePool&) = delete;

            FramePool(FramePool&& other) noexcept
                : commandPools(std::move(other.commandPools)),
                  records(std::move(other.records)),
                  fence(other.fence),
                  id(other.id.load()) {}

            FramePool& operator=(FramePool&& other) noexcept {
                commandPools = std::move(other.commandPools);
                records = std::move(other.records);
                fence = other.fence;
                id.store(other.id.load());
                return *this;
            }
        };
    public:
        Make = []{ return Singleton<CommandCollector>::make_ptr(); };
        CommandCollector();
        ~CommandCollector() = default;

        auto bind(const CommandFn& fn) -> void;
        auto bind(const std::uint32_t& order, const std::uint32_t& grow, const CommandFn& fn) -> void;
        auto submit() -> void;
        auto clear() const -> void;
    private:
        auto expand(SafeCommandPool& safe_command_pool) const -> void;
    private:
        ev::pVkSetupContext ct{};
        uint32_t *index = nullptr;
        vot::vector<FramePool> frames;
    };

} // rhi2


namespace yic {
    inline rhi2::CommandCollector* command2;
}

#endif //VKCELSHADINGRENDERER_CommandCollector_H