//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_SYSTEMHUB_H
#define VKCELSHADINGRENDERER_SYSTEMHUB_H

#include "EventHandle.h"

namespace hide {

    class SystemHub{
        template<typename T>
        struct function_traits : public function_traits<decltype(&T::operator())>{};

        template<typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const>{
            using args_tuple = std::tuple<Args...>;
        };

        struct GlobalEntry{
            std::any any;
            mutable oneapi::tbb::spin_rw_mutex rwMutex;
        };
    public:

        template<typename Func>
        auto subscribe(Func&& func) -> void{
            using function_type = function_traits<Func>;
            using event_type_raw = typename std::tuple_element_t<0, typename function_type::args_tuple>;
            using event_type = typename std::remove_const_t<typename std::remove_reference_t<event_type_raw>>;

            registerEvent<event_type>()->subscribe(func);
        }

        template<typename Func>
        auto subscribePolling(Func&& func) -> void{
            using function_type = function_traits<Func>;
            using event_type_raw = typename std::tuple_element_t<0, typename function_type::args_tuple>;
            using event_type = typename std::remove_const_t<typename std::remove_reference_t<event_type_raw>>;

            registerEvent<event_type>()->subscribePolling(func);
        }

        template<typename Event>
        auto poll() -> void{
            registerEvent<Event>()->poll();
        }

        template<typename Event>
        auto publish(Event&& event) -> void{
            registerEvent<Event>()->publish(event);
        }

        template<typename Event>
        auto publishPolling(Event&& event) -> void{
            registerEvent<Event>()->publishPolling(event);
        }

//        template<typename Event>
//        auto publish_(Event&& event) -> void{
//            registerEvent<Event>()->publish_(event);
//        }

        template<class Event>
        auto publishAsync(Event&& event) -> void{
            eventGroup.run([ event = std::forward<Event>(event)]{
                registerEvent<Event>()->publish(event);
            });
        }



        template<typename Event>
        auto enqueueEvent(Event&& event) -> void{
            auto ev = registerEvent<Event>();
            ev->add(event);
            eventHandles.push(ev);
        }

        template<typename Event>
        auto enqueueLatestEvent(Event&& event) -> void{
            auto ev = registerEvent<Event>();
            ev->update(event);
            std::type_index typeIndex{typeid(event)};
            if (uniqueTypeSet.find(typeIndex) == uniqueTypeSet.end()){
                uniqueHandles.push(ev);
                uniqueTypeSet.insert(typeIndex);
            }
        }

        template<typename Event>
        auto setEvent(Event&& event) -> void{
            registerEvent<Event>()->update(event);
        }

        template<typename Event>
        auto valEvent() -> Event{
            return registerEvent<Event>()->val();
        }

        auto process() -> void{
            while(!eventHandles.empty()){
//                auto handle = eventHandles.front();
//                handle->execute();
//                eventHandles.pop();
                IEventHandle* handle;
                while(eventHandles.try_pop(handle)){
                    handle->execute();
                }
            }
            while(!uniqueHandles.empty()){
//                auto handle = uniqueHandles.front();
//                handle->executeLatest();
//                uniqueHandles.pop();
                IEventHandle* handle;
                while(eventHandles.try_pop(handle)){
                    handle->executeLatest();
                }
            }
            uniqueTypeSet.clear();
        }

        //--------------------------------------------------------------------------------//
        //--------------------------------------------------------------------------------//
        //--------------------------------------------------------------------------------//

        template<typename T>
        auto sto(const T& instance, const vot::string& id = {}) -> void {
            auto& ent = storages[std::type_index(typeid(T))][id];

            {
                oneapi::tbb::spin_rw_mutex::scoped_lock lock(ent.rwMutex, true);

                auto& sto = ent.any;
                if (sto.has_value()){
                    updatePart(std::any_cast<T&>(sto), instance);

                } else { sto = instance; };
            }
        }

        template<typename T>
        auto val(const vot::string& id = {}) -> T&{
            auto it = storages.find(std::type_index(typeid(T)));

            if (it != storages.end()){
                auto& map = it->second;
                auto idt = map.find(id);
                if (idt != map.end()){
                    auto& ent = idt->second;
                    oneapi::tbb::spin_rw_mutex::scoped_lock lock(ent.rwMutex, false);
                    return std::any_cast<T&>(ent.any);
                }
            }

            throw std::runtime_error("The struct is not initialized or the id does not exist.");
        };

        template<typename T>
        auto lockedVal(const vot::string &id = {}) -> Locked<T, oneapi::tbb::spin_rw_mutex>{
            auto it = storages.find(std::type_index(typeid(T)));

            if (it != storages.end()){
                auto& map = it->second;
                auto idt = map.find(id);
                if (idt != map.end()){
                    auto& ent = idt->second;
                    return LockedRef<T>(std::any_cast<T&>(ent.any), ent.rwMutex);
                }
            }
            throw std::runtime_error("The struct is not initialized or the id does not exist.");
        }

    private:
        template<typename T>
        auto updatePart(T& exist, const T& update){
            boost::hana::for_each(boost::hana::keys(exist), [&](auto key){
                auto& oldV = boost::hana::at_key(exist, key);
                const auto& newV = boost::hana::at_key(update, key);

                using oldT = std::decay_t<decltype(oldV)>;
                using newT = std::decay_t<decltype(newV)>;

                if constexpr (std::is_same_v<oldT, newT>) {
                    if (updateCondition(newV)){
                        oldV = newV;
                    }
                }

            });
        }

        template<typename T>
        requires std::is_pointer_v<T>
        auto updateCondition(const T& newVal) -> bool {
            return newVal != nullptr;
        }

        template<typename T>
        requires (std::is_same_v<T, std::shared_ptr<typename T::element_type>> ||
                  std::is_same_v<T, std::unique_ptr<typename T::element_type>>)
        auto updateCondition(const T& newVal) -> bool {
            return newVal != nullptr;
        }

        template<typename T>
        requires (!std::is_pointer_v<T> && requires (T t) { typename T::value_type; } && !requires (T t) { {t.operator bool() } -> std::same_as<bool>; })
        auto updateCondition(const T& newVal) -> bool {
            if constexpr (std::is_same_v<T, std::optional<typename T::value_type>>) {
                return newVal.has_value();
            } else {
                return newVal != T{};
            }
        }

        template<typename T>
        requires requires (T t) { {t.operator bool() } -> std::same_as<bool>; }
        auto updateCondition(const T& newVal) -> bool{
            return newVal.operator bool();
        }

    private:
        oneapi::tbb::task_group eventGroup;
        vot::unordered_map<std::type_index, vot::unordered_map<vot::string, GlobalEntry>> storages;
//        vot::queue<IEventHandle*> eventHandles;
//        vot::queue<IEventHandle*> uniqueHandles;
        oneapi::tbb::concurrent_queue<IEventHandle*> eventHandles;
        oneapi::tbb::concurrent_queue<IEventHandle*> uniqueHandles;
        vot::unordered_set<std::type_index> uniqueTypeSet;

//        vot::unordered_map<std::type_index, std>
    };

}

namespace yic{
    inline auto& systemHub = Singleton<hide::SystemHub>::make();
}


#endif //VKCELSHADINGRENDERER_SYSTEMHUB_H
