//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTBUS_H
#define VKCELSHADINGRENDERER_EVENTBUS_H

#include "EventTypes.h"
#include "Event.h"
#include "Engine/Utils/ThreadPool.h"

namespace yic {

    class EventBus {
    public:
        vkGet auto get = [](size_t nums = 4){ return Singleton<EventBus>::get(4);};

        template<typename Event>
        using EventHandler = std::function<void(const Event&)>;

        explicit EventBus(size_t numThreads) : mThreadPool(numThreads){};

        template<typename Event>
        static void subscribe(const EventHandler<Event>& handler, const std::string& id = "default"){
            auto type = std::type_index(typeid(Event));
            auto inst = get();
            std::lock_guard<std::mutex> lock(get()->mMutex);

            inst->mHandlers[type][id].emplace_back([handler](const std::any& event){
                handler(std::any_cast<const Event&>(event));
            });
        }

        template<typename Handler>
        static void subscribeAuto(Handler&& handler, const std::string& id = "default"){
            using EventType = typename std::decay<typename function_traits<decltype(&Handler::operator())>::arg0_type>::type;
            subscribe<EventType>(std::forward<Handler>(handler), id);
        }

        template<typename Event>
        static void publish(const Event& event, const std::string& id = "default"){
            auto inst = get();
            auto type = std::type_index(typeid(Event));

            std::lock_guard<std::mutex> lock(inst->mMutex);

            std::any& storedEvent = inst->mState[type][id];

            if (!storedEvent.has_value()) {
                storedEvent = event;
            } else {
                inst->updateOptional(std::any_cast<Event&>(storedEvent), event);
            }

            auto& handlers = inst->mHandlers[type][id];
            for (auto& handler : handlers) {
                inst->mThreadPool.enqueue([handler, eventCaptured = std::any(event)]() {
                    handler(eventCaptured);
                });
            }
        }

        struct Get {
#define parm_id const std::string& id = "default"

            static auto vkWindowContext(parm_id) {
                return getState<et::WindowContext>(id);
            }

            static auto vkInitContext(parm_id) {
                return getState<et::vkInitContext>(id);
            }

            static auto vkDeviceContext(parm_id) {
                return getState<et::vkDeviceContext>(id);
            }

            static auto vkSwapchainContext(parm_id){
                return getState<et::vkSwapchainContext>(id);
            }

#undef parm_id
        };


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


        template<typename Event>
        void updateOptional(Event& exist, const Event& updates){
            boost::hana::for_each(boost::hana::keys(exist), [&](auto key){
                auto& oldVal = boost::hana::at_key(exist, key);
                const auto& newVal = boost::hana::at_key(updates, key);

                using oldValType = decltype(oldVal);
                using newValType = std::decay_t<decltype(*newVal)>;

                if constexpr (std::is_same_v<oldValType, std::optional<newValType>>){
                    if (newVal.has_value())
                        oldVal = newVal;
                } else if constexpr (std::is_same_v<oldValType, std::shared_ptr<newValType>>) {
                    if (newVal != nullptr)
                        oldVal = newVal;
                } else if constexpr (std::is_same_v<oldValType, std::unique_ptr<newValType>>){
                    if (auto ptr = newVal.get(); ptr != nullptr){
                        if (!oldVal){
                            oldVal = std::make_unique<newValType>(*ptr);
                        } else{
                            *(oldVal.get()) = *ptr;
                        }
                    }
                } else {
                    oldVal = newVal;
                }
            });
        }

        template<typename T>
        static T getState(const std::string& id = "default") {
            auto inst = get();
            std::lock_guard<std::mutex> lock(get()->mStateMutex);

            auto typeIt = inst->mState.find(std::type_index(typeid(T)));
            if(typeIt != inst->mState.end()){
                auto& idMap = typeIt->second;
                auto idIt = idMap.find(id);
                if (idIt != idMap.end()){
                    return std::any_cast<T>(idIt->second);
                }
            }
            throw std::runtime_error("state not found for the requested type.");
        }


        std::unordered_map<std::type_index, std::unordered_map<std::string, std::vector<std::function<void(const std::any&)>>>> mHandlers;
        std::unordered_map<std::type_index, std::unordered_map<std::string, std::any>> mState;
        std::mutex mMutex, mStateMutex;
        ThreadPool mThreadPool;
    };

} // yic




#endif //VKCELSHADINGRENDERER_EVENTBUS_H



/// 可用于共享指针等等，不强制指针

//        template<typename Event>
//        static void subscribe(const EventHandler<Event>& handler){
//            auto type = std::type_index(typeid(Event));
//            std::lock_guard<std::mutex> lock(get()->mMutex);
//            get()->mHandlers[type].emplace_back([handler](const std::any& event){
//                handler(std::any_cast<const Event&>(event));
//            });
//        }
//
//        template<typename Handler>
//        static void subscribeAuto(Handler&& handler){
//            using EventType = typename std::decay<typename function_traits<decltype(&Handler::operator())>::arg0_type>::type;
//            subscribe<EventType>(std::forward<Handler>(handler));
//        }
//
//        template<typename Event>
//        static void publish(const Event& event){
//            auto inst = get();
//            std::lock_guard<std::mutex> lock(inst->mMutex);
//            std::any& storedEvent = inst->mState[std::type_index(typeid(Event))];
//
//            if (!storedEvent.has_value()) {
//                storedEvent = event;
//            } else {
//                inst->updateOptional(std::any_cast<Event&>(storedEvent), event);
//            }
//
//            auto type = std::type_index(typeid(Event));
//            auto& handlers = inst->mHandlers[type];
//            for (auto& handler : handlers) {
//                inst->mThreadPool.enqueue([handler, eventCaptured = std::any(event)]() {
//                    handler(eventCaptured);
//                });
//            }
//        }

//        template<typename T>
//        static T getState(const std::string& id = "default") {
//            std::lock_guard<std::mutex> lock(get()->mStateMutex);
//            auto it = get()->mState.find(std::type_index(typeid(T)));
//            if(it != get()->mState.end()){
//                return std::any_cast<T>(it->second);
//            }
//            throw std::runtime_error("state not found for the requested type.");
//        }

//        std::unordered_map<std::type_index, std::vector<std::function<void(const std::any&)>>> mHandlers;
//        std::unordered_map<std::type_index, std::any> mState;

/// 只能用于指针，有一定限制，暂未发现问题

//        template<typename Event>
//        static void subscribe(const EventHandler<Event>& handler){
//            auto type = std::type_index(typeid(Event));
//            std::lock_guard<std::mutex> lock(get()->mMutex);
//            get()->mHandlers[type].emplace_back([handler](const void* event){
//                handler(*static_cast<const Event*>(event));
//            });
//        }



//        template<typename Event>
//        static void publish(const Event& event){
//            auto inst = get();
//            auto type = std::type_index(typeid(Event));
//            std::lock_guard<std::mutex> lock(inst->mMutex);
//            auto& storedEvent = inst->mState[std::type_index(typeid(event))];
//
//            if (storedEvent.has_value()){
//                inst->updateOptional(std::any_cast<Event&>(storedEvent), event);
//            } else {
//                storedEvent = event;
//            }
//
//            if (inst->mHandlers.count(type)){
//                for(const auto& handler : inst->mHandlers.at(type)){
//                    inst->mThreadPool.enqueue([handler, event]{
//                      handler(&event);
//                    });
//                }
//            }
//        }


// std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> mHandlers;
