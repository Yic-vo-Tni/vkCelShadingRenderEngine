//
// Created by lenovo on 6/4/2024.
//

#ifndef VKCELSHADINGRENDERER_TASKBUS_H
#define VKCELSHADINGRENDERER_TASKBUS_H

#include <utility>
#include "Engine/Utils/Log.h"
#include "Engine/Core/DispatchSystem/Task/TaskTypes.h"

namespace yic {

    class TaskBus {
        using task = std::function<void()>;

        task specialTask = [](){};
    public:
        vkGet auto get = [](){ return Singleton<TaskBus>::get();};

        explicit TaskBus() {};

        template<typename EnumType>
        static void registerTask(EnumType type, task t, const std::string& id = {}){
            auto inst = get();
            auto typeIndex = std::type_index(typeid(type));
            inst->mTasks[typeIndex][id][static_cast<int>(type)].push_back(std::move(t));
        }

        template<typename EnumType>
        static void executeTask(const std::string& id = {}, const bool& parallel = false){
            executeTaskSpecific_wrapper<EnumType>(std::nullopt, id, parallel);
        }

        template<typename EnumType>
        static void executeTaskSpecific(EnumType type, const std::string& id = {}, const bool& parallel = false){
            executeTaskSpecific_wrapper<EnumType>(type, id, parallel);
        }
        template<typename EnumType>
        static void executeTaskSpecific(std::vector<EnumType> type){
            executeTaskSpecific_wrapper<EnumType>(type);
        }

        template<typename EnumType>
        static void executeTaskSpecific_wrapper(std::optional<std::variant<EnumType, std::vector<EnumType>>> specificTasks, const std::string& id = {}, const bool& parallel = false) {
            auto inst = get();
            auto typeIndex = std::type_index(typeid(EnumType));

            std::vector<std::pair<task, bool>> taskToExecute{};
            {
                if (inst->mTasks.find(typeIndex) != inst->mTasks.end()){
                    for(auto& [key, taskList] : inst->mTasks[typeIndex][id]){
                        bool execute = true;
                        if (specificTasks.has_value()) {
                            auto& val = specificTasks.value();
                            if (std::holds_alternative<EnumType>(val)) {
                                execute = (key == static_cast<int>(std::get<EnumType>(val)));
                            } else {
                                auto& vec = std::get<std::vector<EnumType>>(val);
                                execute = std::find(vec.begin(), vec.end(), static_cast<EnumType>(key)) != vec.end();
                            }
                        }
                        if (execute) {
                            for(auto& t : taskList){
                                taskToExecute.emplace_back(t, false);
                            }
                        }
                        if (parallel) taskToExecute.emplace_back(inst->specialTask, true);
                    }
                }
            }

            auto group = std::make_unique<tbb::task_group>();
            for(auto& t : taskToExecute){
                if (parallel){
                    if (t.second){
                        group->wait();
                        group = std::make_unique<tbb::task_group>();
                    } else{
                        group->run(t.first);
                    }
                } else{
                    t.first();
                }
            }
            if (parallel) {
                group->wait();
            }
        }

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
                    vkError("Shader hot reload successfully for shader: " + fileName);
                }
            }

            inst->mShaderUpdatePaths.clear();
        }

    private:
        tbb::task_group mGroup{};
        tbb::concurrent_unordered_map<std::type_index, tbb::concurrent_unordered_map<std::string, tbb::concurrent_map<int, std::vector<task>>>> mTasks;
        std::unordered_map<std::string, task> mShaderTasks;
        std::vector<std::string> mShaderUpdatePaths;
        std::mutex mShaderMutex;

    };

} // yic

#endif //VKCELSHADINGRENDERER_TASKBUS_H
