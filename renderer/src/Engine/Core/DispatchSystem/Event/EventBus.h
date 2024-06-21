//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTBUS_H
#define VKCELSHADINGRENDERER_EVENTBUS_H

#include "EventTypes.h"
#include "Event.h"
#include "Engine/Utils/TypeConcepts.h"

namespace yic {

    class EventBus {
        struct EventDate{
            std::any states;
            mutable tbb::speculative_spin_rw_mutex mutex_speculative;
            mutable tbb::queuing_rw_mutex mutex_queuing;
            std::atomic<bool> deferred;
            std::vector<std::function<void(const std::any&)>> handlers;
        };

    public:
        vkGet auto get = [](){ return Singleton<EventBus>::get();};

        template<typename Event>
        using EventHandler = std::function<void(const Event&)>;

        explicit EventBus() {};

        template<typename Handler>
        static void subscribeAuto(Handler&& handler, const std::string& id = {}){
            using EventType = typename std::decay<typename function_traits<decltype(&Handler::operator())>::arg0_type>::type;
            subscribe<EventType>(std::forward<Handler>(handler), id);
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
        static void update(const Event& event, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(event));

            auto &eventDate = inst->mEventDates[type][id];
            {
                tbb::speculative_spin_rw_mutex::scoped_lock lock(eventDate.mutex_speculative, true);
                auto &storedEvent = eventDate.states;

                if (!storedEvent.has_value()) {
                    storedEvent = event;
                } else {
                    inst->updateOptional(std::any_cast<Event &>(storedEvent), event);
                }
            }

        }

#define default_parm_id const std::string& id = {}
#define parm_id const std::string& id

        struct Get {
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

            static auto test(default_parm_id){
                return getState<et::test>(id);
            }
        };

        struct GetRef_scoped{
            static auto& test(default_parm_id){
//                return getStateRef<et::test>(id).get();
                return getStateRef_f<et::test>(id);
            }
        };

#undef default_parm_id
#undef parm_id


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

        template<typename Event>
        static void publish_impl(const Event& event, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(Event));

            update(event, id);

            auto& handlers = inst->mEventDates[type][id].handlers;
            auto& deferred = inst->mEventDates[type][id].deferred;
            deferred.store(true);
            for (auto& handler : handlers) {
                inst->mGroup.run([handler, eventCaptured = std::any(event)](){
                    handler(eventCaptured);
                });
                inst->mGroup.wait();
            }
        }

        template<typename T>
        static T getState(const std::string& id = {}) {
            auto inst = get();

            auto typeIt = inst->mEventDates.find(std::type_index(typeid(T)));
            if(typeIt != inst->mEventDates.end()){
                auto& idMap = typeIt->second;
                auto idIt = idMap.find(id);
                if (idIt != idMap.end()){
                    const auto& eventDate = idIt->second;
                    tbb::speculative_spin_rw_mutex::scoped_lock lock(eventDate.mutex_speculative, false);
                    return std::any_cast<T>(eventDate.states);
                }
            }
            throw std::runtime_error("state not found for the requested type.");
        }

        template<typename T>
        static T& getStateRef_f(const std::string& id = {}){
            return getStateRef<T>(id).get();
        }

        template<typename T>
        static LockedState<T> getStateRef(const std::string& id = {}) {
            auto inst = get();
            auto typeIt = inst->mEventDates.find(std::type_index(typeid(T)));
            if (typeIt != inst->mEventDates.end()) {
                auto& idMap = typeIt->second;
                auto idIt = idMap.find(id);
                if (idIt != idMap.end()) {
                    auto& eventDate = idIt->second;
                    return LockedState<T>{std::any_cast<T&>(eventDate.states), eventDate.mutex_queuing};
                }
            }
            throw std::runtime_error("State not found for the requested type");
        }

        template<typename Event>
        static void subscribe(const EventHandler<Event>& handler, const std::string& id = {}){
            auto type = std::type_index(typeid(Event));
            auto inst = get();

            inst->mEventDates[type][id].handlers.emplace_back([handler](const std::any& event){
                handler(std::any_cast<const Event&>(event));
            });
        }

        template<typename Event>
        static void subscribeDeferred(const EventHandler<Event>& handler, const std::string& id = {}){
            auto inst = get();
            auto type = std::type_index(typeid(Event));

            if (inst->mEventDates[type][id].deferred && inst->mEventDates[type].find(id) != inst->mEventDates[type].end()){
                auto& event = std::any_cast<const Event&>(inst->mEventDates[type][id].states);
                handler(event);
                inst->mEventDates[type][id].deferred.store(false);
            }
        }


    private:
        tbb::task_group mGroup;
        tbb::concurrent_unordered_map<std::type_index, tbb::concurrent_unordered_map<std::string, EventDate>> mEventDates;
    };

} // yic




#endif //VKCELSHADINGRENDERER_EVENTBUS_H



