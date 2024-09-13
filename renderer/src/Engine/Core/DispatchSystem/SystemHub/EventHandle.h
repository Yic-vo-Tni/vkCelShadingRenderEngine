//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTHANDLE_H
#define VKCELSHADINGRENDERER_EVENTHANDLE_H

namespace Hide{

//    template<typename T>
//    std::unique_ptr<char, void (*)(void*)> real_name() {
//        char* demangled_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
//        return std::unique_ptr<char, void (*)(void*)>(demangled_name, std::free);
//    }

    class IEventHandle{
    public:
        virtual auto execute() -> void = 0;
        virtual auto executeLatest() -> void = 0;
        virtual ~IEventHandle() = default;
    };


    template<typename Event>
    class EventHandle : public IEventHandle{
    protected:
        using Handler = std::function<void(const Event&)>;
    public:
        auto subscribe(Handler handler) -> void {
            handlers.push_back(std::move(handler));
        }

        auto publish(const Event& e) -> void {
            event = e;
            for(auto& handler : handlers){
                handler(e);
            }
        }

        auto update(const Event& e) -> void{
            event = e;
        }

        auto add(const Event& e) -> void{
            queue.push(e);
            event = e;
        }

        void execute() override{
            while(!queue.empty()){
                auto& e = queue.front();
                for(auto& handler : handlers){
                    handler(e);
                }
                queue.pop();
            }
        }
        void executeLatest() override{
            for(auto& handler : handlers){
                handler(event.value());
            }
        }

        auto val() -> Event{
            if (!event.has_value()){
                vkError("the event: {0} has not value!", typeid(event).name());
            }
            return event.value();
        }

    protected:
        std::optional<Event> event;
        vot::queue<Event> queue;
        vot::vector<Handler> handlers;
    };

    template<typename Event>
    EventHandle<Event>* registerEvent(){
        static EventHandle<Event> e;
        return &e;
    }


}

#endif //VKCELSHADINGRENDERER_EVENTHANDLE_H
