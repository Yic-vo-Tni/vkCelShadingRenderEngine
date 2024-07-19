//
// Created by lenovo on 7/18/2024.
//

#ifndef VKCELSHADINGRENDERER_SHADERHOTRELOADER_H
#define VKCELSHADINGRENDERER_SHADERHOTRELOADER_H

namespace yic{

    class ShaderHotReLoader{
        using task = std::function<void()>;
    public:
        vkGet auto get = []{ return Singleton<ShaderHotReLoader>::get(); };

        static void registerShaderFileTask(const std::string& shaderPath, task t){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mShaderMutex);
            inst->mShaderTasks[shaderPath] = std::move(t);
        }

        static void recordShaderUpdatePath(const std::string& shaderPath){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mShaderMutex);
            inst->mShaderUpdatePaths.emplace_back(shaderPath);
        }

        static void executeShaderTask(){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mShaderMutex);

            for(auto& p : inst->mShaderUpdatePaths){
                if (inst->mShaderTasks.find(p) != inst->mShaderTasks.end()) {
                    inst->mGroup.run_and_wait(inst->mShaderTasks[p]);
                    auto fileName = std::filesystem::path(p).filename().string();
                    vkWarn("Shader hot reload successfully for shader: " + fileName);
                }
            }

            inst->mShaderUpdatePaths.clear();
        }


    private:
        tbb::task_group mGroup{};
        std::unordered_map<std::string, task> mShaderTasks;
        std::vector<std::string> mShaderUpdatePaths;
        std::mutex mShaderMutex;
    };



}

#endif //VKCELSHADINGRENDERER_SHADERHOTRELOADER_H
