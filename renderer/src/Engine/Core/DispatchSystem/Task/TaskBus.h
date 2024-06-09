//
// Created by lenovo on 6/4/2024.
//

#ifndef VKCELSHADINGRENDERER_TASKBUS_H
#define VKCELSHADINGRENDERER_TASKBUS_H

#include "Engine/Core/DispatchSystem/Task/TaskTypes.h"

namespace yic {

    class TaskBus {
    public:
        using task = std::function<void()>;
        vkGet auto get = [](){ return Singleton<TaskBus>::get();};

        template<typename EnumType>
        static void registerTask(EnumType type, task t){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mMutex);
            auto typeIndex = std::type_index(typeid(type));
            inst->mTasks[typeIndex][static_cast<int>(type)].push_back(t);
        }

        template<typename EnumType>
        static void executeTask(){
            auto inst = get();
            std::lock_guard<std::mutex> lock(inst->mMutex);
            auto typeIndex = std::type_index(typeid(EnumType));
            if (inst->mTasks.find(typeIndex) != inst->mTasks.end()){
                for(auto& [key, taskList] : inst->mTasks[typeIndex]){
                    for(auto& t : taskList){
                        t();
                    }
                }
            }
        }

    private:
        std::unordered_map<std::type_index, std::map<int, std::vector<task>>> mTasks;
        std::mutex mMutex;
    };

} // yic

#endif //VKCELSHADINGRENDERER_TASKBUS_H
