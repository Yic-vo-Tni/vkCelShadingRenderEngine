//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTHANDLE_H
#define VKCELSHADINGRENDERER_EVENTHANDLE_H
#include <barrier>

namespace hide {


    class IEventHandle{
    public:
        virtual auto execute() -> void = 0;
        virtual auto executeLatest() -> void = 0;
        virtual ~IEventHandle() = default;
    };


    template<typename Event>
    class EventHandle final : public IEventHandle{
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

        auto setParticipantCount(const size_t count) {
            participantCount = count;
            syncBarrier = std::make_unique<std::barrier<>>(participantCount);
        }

        auto subscribePolling(Handler handler) -> void{
            pollingHandlers.push_back(std::move(handler));
        }


        auto publishPolling(const Event& e) -> void{
            event = e;
            queue.push(e);
        }

        auto poll() -> void {
         //   Event e;
            while (!queue.empty()) {
                auto &e = queue.front();

                syncBarrier->arrive_and_wait();

                if (bool expected = false; executed.compare_exchange_strong(expected, true)) {
                    for (auto &handler: pollingHandlers) {
                        handler(e);
                    }
                    queue.pop();
                }

                syncBarrier->arrive_and_wait();

                executed.store(false);
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
                yic::logger->error("the event: {0} has not value!", typeid(event).name());
            }
            return event.value();
        }

    protected:
        std::atomic<bool> executed{false};
        std::optional<Event> event;
        vot::queue<Event> queue;
        //oneapi::tbb::concurrent_bounded_queue<Event> queue;
        vot::vector<Handler> handlers;
        vot::vector<Handler> pollingHandlers;
        std::unique_ptr<std::barrier<>> syncBarrier;
        size_t participantCount = 0;
    };

    template<typename Event>
    EventHandle<Event>* registerEvent(){
        static EventHandle<Event> e;
        return &e;
    }


} // hide

#endif //VKCELSHADINGRENDERER_EVENTHANDLE_H
