//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_SYSTEMHUB_H
#define VKCELSHADINGRENDERER_SYSTEMHUB_H

#include "EventHandle.h"

namespace hide {

    class SystemHub{
        template<typename T>
        struct function_traits : function_traits<decltype(&T::operator())>{};

        template<typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const>{
            using args_tuple = std::tuple<Args...>;
        };

        struct Entry {
            mutable oneapi::tbb::spin_rw_mutex rwMutex;
        };
        template<typename T>
        struct EntryHandle : Entry {
            std::optional<T> any;
        };
    public:

        template<typename Func>
        auto sub(Func&& func) -> void{
            using function_type = function_traits<Func>;
            using event_type_raw = std::tuple_element_t<0, typename function_type::args_tuple>;
            using event_type = std::remove_const_t<std::remove_reference_t<event_type_raw>>;

            registerEvent<event_type>()->sub(func);
        }

        template<typename Event>
        auto pub(Event&& event) -> void{
            registerEvent<Event>()->pub(event);
        }

        template<class Event>
        auto pub_async(Event&& event) -> void{
            eventGroup.run([ event = std::forward<Event>(event)]{
                registerEvent<Event>()->pub(event);
            });
        }

        template<typename Func>
        auto sub_queued(const size_t count, Func&& func) -> void{
            using function_type = function_traits<Func>;
            using event_type_raw = std::tuple_element_t<0, typename function_type::args_tuple>;
            using event_type = std::remove_const_t<std::remove_reference_t<event_type_raw>>;

            registerEvent<event_type>()->sub_queued(count, func);
        }

        template<typename Event>
        auto pub_enqueue(Event&& event) -> void{
            registerEvent<Event>()->pub_enqueue(event);
        }

        template<typename... Events>
        auto dispatch() -> void {
            (registerEvent<Events>()->dispatch(), ...);
        }

        template<typename Event>
        auto setEvent(Event&& event) -> void{
            registerEvent<Event>()->up(event);
        }

        template<typename Event>
        auto valEvent() -> Event{
            return registerEvent<Event>()->va();
        }

        auto process() -> void{
            // while(!eventHandles.empty()){
            //     IEventHandle* handle;
            //     while(eventHandles.try_pop(handle)){
            //         handle->execute();
            //     }
            // }
            // while(!uniqueHandles.empty()){
            //     IEventHandle* handle;
            //     while(eventHandles.try_pop(handle)){
            //         handle->executeLatest();
            //     }
            // }  FIXME TODO
        }

        //--------------------------------------------------------------------------------//
        //--------------------------------------------------------------------------------//
        //--------------------------------------------------------------------------------//



        template<typename T>
        auto sto(T &&instance, const vot::string &id = {}) -> void {
            using U = std::decay_t<T>;
            auto &uEnt = Entries[std::type_index(typeid(U))][id];

            if (!uEnt) { uEnt = std::make_unique<EntryHandle<U> >(); }

            {
                auto *ptr = static_cast<EntryHandle<U> *>(uEnt.get());
                oneapi::tbb::spin_rw_mutex::scoped_lock lock(ptr->rwMutex, true);
                ptr->any = std::forward<T>(instance);
            }
        }

        template<typename T>
        auto sto(const T& instance, const vot::string& id = {}) -> void {
            auto& uEnt = Entries[std::type_index(typeid(T))][id];

            if (!uEnt) { uEnt = std::make_unique<EntryHandle<T>>(); }

            {
                auto *ptr = static_cast<EntryHandle<T> *>(uEnt.get());
                oneapi::tbb::spin_rw_mutex::scoped_lock lock(ptr->rwMutex, true);
                ptr->any = instance;
            }
        }

        template<typename T>
        auto va(const vot::string& id = {}) ->T& {
            if (const auto it = Entries.find(std::type_index(typeid(T))); it != Entries.end()) {
                if (const auto idt = it->second.find(id); idt != it->second.end()) {
                    auto* ent = static_cast<EntryHandle<T>*>(idt->second.get());

                    oneapi::tbb::spin_rw_mutex::scoped_lock lock(ent->rwMutex, false);
                    if (!ent->any) {
                        throw std::runtime_error("The struct is not initialized or the id does not exist.");
                    }
                    return *ent->any;
                }
            }
            throw std::runtime_error("The struct is not initialized or the id does not exist.");
        }

        template<typename T>
        auto vaL(const vot::string& id = {}) -> Locked<T, oneapi::tbb::spin_rw_mutex>{
            if (const auto it = Entries.find(std::type_index(typeid(T))); it != Entries.end()) {
                if (const auto idt = it->second.find(id); idt != it->second.end()) {
                    auto* ent = static_cast<EntryHandle<T>*>(idt->second.get());
                    return Locked<T, oneapi::tbb::spin_rw_mutex>(*ent->any, ent->rwMutex);
                }
            }
            throw std::runtime_error("The struct is not initialized or the id does not exist.");
        }

    private:
        oneapi::tbb::task_group eventGroup;

        vot::unordered_map<std::type_index, vot::unordered_map<vot::string, std::unique_ptr<Entry>>> Entries;
    };

}

namespace yic{
    inline auto& systemHub = Singleton<hide::SystemHub>::make();
}


#endif //VKCELSHADINGRENDERER_SYSTEMHUB_H

