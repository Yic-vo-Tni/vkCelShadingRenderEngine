//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_LOG_H
#define VKCELSHADINGRENDERER_LOG_H

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


struct vkCreateInvoker {
    explicit vkCreateInvoker(std::string description, spdlog::level::level_enum level) : mDescription(std::move(description)),
                                                                                         mLevel(level) {};

    vkCreateInvoker() = default;

    template<typename Func>
    auto operator=(Func &&func) {
        try {
            auto r = func();

            if_debug {
                switch (mLevel) {
                    case spdlog::level::info:
                        vkInfo("{0}, successfully", mDescription);
                        break;
                    case spdlog::level::warn:
                        vkWarn("{0} successfully", mDescription);
                        break;
                    case spdlog::level::err:
                        vkError("{0} successfully", mDescription);
                        break;
                    default:
                        vkTrance("{0} successfully", mDescription);
                }
            };

            return r;
        } catch (const vk::SystemError &e) {
            if_debug vkError("failed to {0}: {1}", mDescription, e.what());
            exit(EXIT_FAILURE);
        }
    }

private:
    std::string mDescription{};
    spdlog::level::level_enum mLevel{};
};

inline vkCreateInvoker vkCreate(const std::string & description = {}, spdlog::level::level_enum level = spdlog::level::info){
    return vkCreateInvoker(description, level);
}

//



#endif //VKCELSHADINGRENDERER_LOG_H
