//
// Created by lenovo on 5/21/2024.
//

#include "Log.h"

namespace yic {

    Log::Log() {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        mLogger = spdlog::stdout_color_mt("vk");
        mLogger->set_level(spdlog::level::trace);
    }

} // yic