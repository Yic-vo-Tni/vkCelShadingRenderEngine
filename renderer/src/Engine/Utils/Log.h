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

inline void try_catch(const std::function<void()> &fun, const std::string &des, spdlog::level::level_enum level){
    try {
        fun();
    } catch (const vk::SystemError &e) {
        if_debug vkError("failed to {0}: {1}", des, e.what());
        exit(EXIT_FAILURE);
    }

    if_debug {
        switch (level) {
            case spdlog::level::info:
                vkInfo("{0}, successfully", des);
                break;
            case spdlog::level::warn:
                vkWarn("{0} successfully", des);
                break;
            case spdlog::level::err:
                vkError("{0} successfully", des);
                break;
            default:
                vkTrance("{0} successfully", des);
        }
    };
}




#endif //VKCELSHADINGRENDERER_LOG_H
