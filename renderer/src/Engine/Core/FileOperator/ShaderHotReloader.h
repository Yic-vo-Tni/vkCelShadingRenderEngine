//
// Created by lenovo on 6/9/2024.
//

#ifndef VKCELSHADINGRENDERER_SHADERHOTRELOADER_H
#define VKCELSHADINGRENDERER_SHADERHOTRELOADER_H

#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Schedulers.h"

namespace yic{

    namespace fs = std::filesystem;

    class ShaderFolderWatcher{
    public:
        vkGet auto get = [](const std::string & path = shader_path){ return Singleton<ShaderFolderWatcher>::get(path);};

        explicit ShaderFolderWatcher(const std::string& path) : mPath([&]{ if (!path.empty()) { return path;}
            throw std::runtime_error("You must init and give a path to shader_directory");
        }()){
            compileShaders();

            for(const auto& file : fs::recursive_directory_iterator(path)){
                if (fs::is_regular_file(file.path())){
                    mPaths[file.path().string()] = fs::last_write_time(file);
                }
            }
        }

        static void start(){
            get()->mShaderReLoader = std::thread(&ShaderFolderWatcher::watch, get());
        }
        static void end(){
            get()->run.exchange(false);
            if (get()->mShaderReLoader.joinable())
                get()->mShaderReLoader.join();
        }

    private:
        static auto compileShaders() -> void {
            std::string shaderPath = shader_path "/..";

            auto cmake_cmd = "cmake -S " + shaderPath + " -B " + shaderPath + "/build";
            auto build_cmd = "cmake --build " + shaderPath + "/build";

            boost::process::system(cmake_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);
            boost::process::system(build_cmd, boost::process::std_out > stdout, boost::process::std_err > stderr);
        }

        static auto convertToSpvPath(const std::string& shaderPath) -> std::string {
            std::filesystem::path path(shaderPath);

            std::string fileName = path.filename().replace_extension(".spv").string();
            std::filesystem::path spvPath = path.parent_path().parent_path() / "shaders_spvs" / fileName;
            auto preferredPath = spvPath.make_preferred().string();
            std::ranges::replace(preferredPath, '\\', '/');

            return preferredPath;
        }

        void watch(){
            while (run.load()){
                std::this_thread::sleep_for(std::chrono::seconds(1));
//                mReLoad = false;

                auto it = mPaths.begin();
                while (it != mPaths.end()){
                    if (!fs::exists(it->first)){
                        vkInfo("File deleted: {0}", it->first);
                        it = mPaths.erase(it);
                    } else{
                        auto current_time = fs::last_write_time(it->first);
                        if (it->second != current_time){
                            it->second = current_time;
                            vkTrance("File updated: " + it->first);
                            compileShaders();
                            //TaskBus::executeShaderTask(convertToSpvPath(it->first));
                            TaskBus::recordShaderUpdatePath(convertToSpvPath(it->first));
//                            mReLoad = true;
                        }
                        ++it;
                    }
                }

                for(const auto& file : fs::recursive_directory_iterator(mPath)){
                    if (fs::is_regular_file(file.path()) && mPaths.find(file.path().string()) == mPaths.end()){
                        mPaths[file.path().string()] = fs::last_write_time(file);
                        vkInfo("New file detected: " + file.path().string());
                    }
                }

//                if (mReLoad)
//                    compileShaders();
            }
        }
    private:
        std::string mPath;
        std::map<std::string, fs::file_time_type> mPaths;
        std::atomic<bool> run{true};
        std::thread mShaderReLoader{};
        //bool mReLoad{false};
    };

}

#endif //VKCELSHADINGRENDERER_SHADERHOTRELOADER_H
