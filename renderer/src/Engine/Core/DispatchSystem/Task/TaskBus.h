//
// Created by lenovo on 6/4/2024.
//

#ifndef VKCELSHADINGRENDERER_TASKBUS_H
#define VKCELSHADINGRENDERER_TASKBUS_H

#include <utility>

#include "Engine/Core/DispatchSystem/Task/TaskTypes.h"
#include "Engine/Utils/ThreadPool.h"

namespace yic {

    class TaskBus {
    public:
        using task = std::function<void()>;
        vkGet auto get = [](size_t nums = 3){ return Singleton<TaskBus>::get(nums);};

        explicit TaskBus(size_t nums) : mThreadPool(nums){};

        template<typename EnumType>
        static void registerTask(EnumType type, task t){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mMutex);
            auto typeIndex = std::type_index(typeid(type));
            inst->mTasks[typeIndex][static_cast<int>(type)].push_back(t);
        }

        template<typename EnumType>
        static void executeTask(const bool& parallel = false){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mMutex);
            auto typeIndex = std::type_index(typeid(EnumType));
            if (inst->mTasks.find(typeIndex) != inst->mTasks.end()){
                for(auto& [key, taskList] : inst->mTasks[typeIndex]){
                    for(auto& t : taskList){
                        parallel ? inst->mThreadPool.enqueue(t) : t();
//                        inst->mThreadPool.enqueue(t);
                    }
                }
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
                    inst->mThreadPool.enqueue(inst->mShaderTasks[p]);
                }
            }
        }

    private:
        std::unordered_map<std::type_index, std::map<int, std::vector<task>>> mTasks;
        std::unordered_map<std::string, task> mShaderTasks;
        std::vector<std::string> mShaderUpdatePaths;
        std::mutex mMutex, mShaderMutex;
        ThreadPool mThreadPool;
    };

} // yic

#endif //VKCELSHADINGRENDERER_TASKBUS_H
