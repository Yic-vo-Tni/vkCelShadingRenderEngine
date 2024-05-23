//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENT_H
#define VKCELSHADINGRENDERER_EVENT_H

#include "EventTypes.h"
#include "Engine/Utils/ThreadPool.h"

namespace yic {

    class EventBus {
    public:
        vkGet auto get = [](size_t nums = 4){ return Singleton<EventBus>::get(4);};

        template<typename Event>
        using EventHandler = std::function<void(const Event&)>;

        explicit EventBus(size_t numThreads) : mThreadPool(numThreads){};

        template<typename Event>
        static void subscribe(const EventHandler<Event>& handler){
            auto type = std::type_index(typeid(Event));
            std::lock_guard<std::mutex> lock(get()->mMutex);
            get()->mHandlers[type].emplace_back([handler](const void* event){
                handler(*static_cast<const Event*>(event));
            });
        }

        template<typename Handler>
        static void subscribeAuto(Handler&& handler){
            using EventType = typename std::decay<typename function_traits<decltype(&Handler::operator())>::arg0_type>::type;
            subscribe<EventType>(std::forward<Handler>(handler));
        }

        template<typename Event>
        static void publish(const Event& event){
            auto type = std::type_index(typeid(Event));
            std::lock_guard<std::mutex> lock(get()->mMutex);
            if (get()->mHandlers.count(type)){
                for(const auto& handler : get()->mHandlers.at(type)){
                    get()->mThreadPool.enqueue([handler, event]{
                        handler(&event);
                    });
                }
            }
        }

    private:
        template<typename T>
        struct function_traits;

        template<typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const> {
            using arg0_type = typename std::tuple_element<0, std::tuple<Args...>>::type;
        };

        template<typename ReturnType, typename... Args>
        struct function_traits<ReturnType(*)(Args...)> {
            using arg0_type = typename std::tuple_element<0, std::tuple<Args...>>::type;
        };

        template<typename ReturnType, typename... Args>
        struct function_traits<std::function<ReturnType(Args...)>> {
            using arg0_type = typename std::tuple_element<0, std::tuple<Args...>>::type;
        };


        std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> mHandlers;
        std::mutex mMutex;
        ThreadPool mThreadPool;
    };

} // yic

#endif //VKCELSHADINGRENDERER_EVENT_H
