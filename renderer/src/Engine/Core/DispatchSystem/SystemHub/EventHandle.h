//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTHANDLE_H
#define VKCELSHADINGRENDERER_EVENTHANDLE_H

namespace Hide{


    template<typename Event>
    class EventHandle{
        using Handler = std::function<void(const Event&)>;
    public:
        auto subscribe(Handler handler) -> void {
            handlers.push_back(std::move(handler));
        }

        auto publish(const Event& e) -> void {
            event = &e;
            for(auto& handler : handlers){
                handler(e);
            }
        }

        auto val() -> Event{
            return *event;
        }

    private:
        vot::vector<Handler> handlers;
        const Event* event = nullptr;
    };

    template<typename Event>
    EventHandle<Event>* registerEvent(){
        static EventHandle<Event> e;
        return &e;
    }




}

#endif //VKCELSHADINGRENDERER_EVENTHANDLE_H
