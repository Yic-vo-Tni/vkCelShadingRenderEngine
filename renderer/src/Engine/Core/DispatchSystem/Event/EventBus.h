//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTBUS_H
#define VKCELSHADINGRENDERER_EVENTBUS_H

#include "EventTypes.h"
#include "Event.h"
#include "Engine/Utils/ThreadPool.h"
#include "Engine/Utils/TypeConcepts.h"

namespace yic {

    class EventBus {
    public:
        vkGet auto get = [](size_t nums = 3){ return Singleton<EventBus>::get(nums);};

        template<typename Event>
        using EventHandler = std::function<void(const Event&)>;

        explicit EventBus(size_t numThreads) : mThreadPool(numThreads){};

        template<typename Event>
        static void subscribe(const EventHandler<Event>& handler, const std::string& id = {}){
            auto type = std::type_index(typeid(Event));
            auto inst = get();

            inst->mHandlers[type][id].emplace_back([handler](const std::any& event){
                handler(std::any_cast<const Event&>(event));
            });
        }

        template<typename Handler>
        static void subscribeAuto(Handler&& handler, const std::string& id = {}){
            using EventType = typename std::decay<typename function_traits<decltype(&Handler::operator())>::arg0_type>::type;
            subscribe<EventType>(std::forward<Handler>(handler), id);
        }

        template<typename Event>
        static void subscribeDeferred(const EventHandler<Event>& handler, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(Event));

            if (inst->mDeferred[type][id] && inst->mState[type].find(id) != inst->mState[type].end()){
                auto& event = std::any_cast<const Event&>(inst->mState[type][id]);
                handler(event);
                inst->mDeferred[type][id].store(false);
            }
        }

        template<typename Handler>
        static void subscribeDeferredAuto(Handler&& handler, const std::string& id = {}){
            using EventType = typename std::decay_t<typename function_traits<decltype(&Handler::operator())>::arg0_type>;
            subscribeDeferred<EventType>(std::forward<Handler>(handler), id);
        }

        template<typename Event, tp::is_string ...Args>
        static void publish(const Event& event, Args...args){
            if constexpr (sizeof ...(Args) == 0){
                publish_impl(event);
            } else {
                (publish_impl(event, args), ...);
            }
        }

        template<typename Event>
        static void publish_impl(const Event& event, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(Event));

            update(event, id);

            auto& handlers = inst->mHandlers[type][id];
            auto& deferred = inst->mDeferred[type][id];
            deferred.store(true);
            for (auto& handler : handlers) {
                inst->mGroup.run([handler, eventCaptured = std::any(event)](){
                    handler(eventCaptured);
                });
                inst->mGroup.wait();
            }
        }


        template<typename Event>
        static void update(const Event& event, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(event));

            std::any &storedEvent = inst->mState[type][id];

            if (!storedEvent.has_value()) {
                storedEvent = event;
            } else {
                inst->updateOptional(std::any_cast<Event &>(storedEvent), event);
            }

        }

        struct Get {
#define default_parm_id const std::string& id = {}
#define parm_id const std::string& id

            static auto vkSetupContext(default_parm_id) {
                return getState<et::vkSetupContext>(id);
            }

            static auto vkRenderContext(parm_id) {
                return getState<et::vkRenderContext>(id);
            }

            static auto glKeyInput(default_parm_id){
                return getState<et::glKeyInput>(id);
            }

            static auto glMouseInput(default_parm_id){
                return getState<et::glMouseInput>(id);
            }

            static auto glCursorPosInput(default_parm_id){
                return getState<et::glCursorPosInput>(id);
            }

            static auto glScrollInput(default_parm_id){
                return getState<et::glScrollInput>(id);
            }

#undef default_parm_id
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

                using oldValType = std::decay_t<decltype(oldVal)>;
                using newValType = std::decay_t<decltype(newVal)>;

                if constexpr (std::is_same_v<oldValType, newValType>){
                    if (updateCondition(newVal)){
                        oldVal = newVal;
                    }
                }
            });
        }

        template<typename T>
        bool updateCondition(const T &newVal) {
            if constexpr (std::is_same_v<T, std::optional<typename T::value_type>>) {
                return newVal.has_value();
            } else if constexpr (std::is_pointer_v<T> || std::is_same_v<T, std::shared_ptr<typename T::element_type>> ||
                                 std::is_same_v<T, std::unique_ptr<typename T::element_type>>) {
                return newVal != nullptr;
            } else if constexpr (std::is_same_v<T, std::vector<typename T::value_type>> ||
                                 std::is_same_v<T, std::map<typename T::key_type, typename T::mapped_type>> ||
                                 std::is_same_v<T, std::unordered_map<typename T::key_type, typename T::mapped_type>> ||
                                 std::is_same_v<T, std::initializer_list<typename T::value_type>> ||
                                 std::is_same_v<T, std::deque<typename T::value_type>>) {
                return !newVal.empty();
            } else {
                return newVal != T{};
            }
        }

        template<typename T>
        static T getState(const std::string& id = {}) {
            auto inst = get();

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


        tbb::concurrent_unordered_map<std::type_index, std::unordered_map<std::string, std::vector<std::function<void(const std::any&)>>>> mHandlers;
        tbb::concurrent_unordered_map<std::type_index, std::unordered_map<std::string, std::any>> mState;
        tbb::concurrent_unordered_map<std::type_index, std::unordered_map<std::string, std::atomic<bool>>> mDeferred;
        tbb::task_group mGroup;
        ThreadPool mThreadPool;
    };

} // yic




#endif //VKCELSHADINGRENDERER_EVENTBUS_H



