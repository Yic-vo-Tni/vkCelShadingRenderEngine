//
// Created by lenovo on 9/1/2025.
//

#ifndef VKCELSHADINGRENDERER_COMMAND_H
#define VKCELSHADINGRENDERER_COMMAND_H

namespace rhi2 {

    class CommandCollector {
        using CommandFn = std::function<void(vot::CommandBuffer&)>;
    public:
        Make = []{ return Singleton<CommandCollector>::make_ptr(); };
        CommandCollector() = default;
        ~CommandCollector() = default;

        auto push(const CommandFn& fn, const uint32_t& priority = 0) -> void;
    private:
        ev::pVkSetupContext ct{};
        vot::vector<CommandFn> mCommands;
    };

} // rhi2

#endif //VKCELSHADINGRENDERER_COMMAND_H