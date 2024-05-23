//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_LOG_H
#define VKCELSHADINGRENDERER_LOG_H

#include "spdlog/include/spdlog/spdlog.h"
#include "spdlog/include/spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/include/spdlog/fmt/ostr.h"
#include "spdlog/include/spdlog/sinks/ansicolor_sink.h"

namespace yic {

    class Log : public nonCopyable{
    public:
        vkGet auto get = [](){ return Singleton<Log>::get();};

        Log();

        inline static auto& GetLogger() { return get()->mLogger; }

    private:
        std::shared_ptr<spdlog::logger> mLogger;
    };

} // yic

#define vkTrance(...)    yic::Log::GetLogger()->trace(__VA_ARGS__)
#define vkInfo(...)      yic::Log::GetLogger()->info(__VA_ARGS__)
#define vkWarn(...)      yic::Log::GetLogger()->warn(__VA_ARGS__)
#define vkError(...)     yic::Log::GetLogger()->error(__VA_ARGS__)


#endif //VKCELSHADINGRENDERER_LOG_H
