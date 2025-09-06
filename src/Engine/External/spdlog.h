//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_SPDLOG_H
#define VKCELSHADINGRENDERER_SPDLOG_H

#include "base.h"
#include "pch.h"

namespace hide{

    class SpdLog : public nonCopyable{
    public:
        Make = []{ return Singleton<SpdLog>::make_ptr(); };
        SpdLog(){
            spdlog::set_pattern("%^[%T] %n: %v%$");
            mLogger = spdlog::stdout_color_mt("vk");
            mLogger->set_level(spdlog::level::trace);
        }

        inline static auto& getLogger() { return make()->mLogger; }

    private:
        std::shared_ptr<spdlog::logger> mLogger;
    };

}

#ifdef NDEBUG
const bool enableDebug = false;
#else
const bool enableDebug = true;
#endif

#define if_debug  if(enableDebug)

namespace yic{
    inline auto& logger = hide::SpdLog::getLogger();
}

struct vkCreateInvoker {
    explicit vkCreateInvoker(std::string description, spdlog::level::level_enum level) : mDescription(std::move(description)),
                                                                                         mLevel(level) {};

    vkCreateInvoker() = default;

    template<typename Func>
    auto operator=(Func &&func) {
        try {
            auto r = func();

            switch (mLevel) {
                case spdlog::level::info:
                    if_debug yic::logger->info("{0}, successfully", mDescription);
                    break;
                case spdlog::level::warn:
                    if_debug yic::logger->warn("{0} successfully", mDescription);
                    break;
                case spdlog::level::err:
                    if_debug yic::logger->error("{0} successfully", mDescription);
                    break;
                default:
                    if_debug yic::logger->trace("{0} successfully", mDescription);
            }

            return r;
        } catch (const vk::SystemError &e) {
            if_debug yic::logger->error("failed to {0}: {1}", mDescription, e.what());
            exit(EXIT_FAILURE);
        }
    }

private:
    std::string mDescription{};
    spdlog::level::level_enum mLevel{};
};

namespace vot {
    inline vkCreateInvoker create(const std::string &description = {}, spdlog::level::level_enum level = spdlog::level::info) {
        return vkCreateInvoker(description, level);
    }
}

#endif //VKCELSHADINGRENDERER_SPDLOG_H
