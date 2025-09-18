//
// Created by lenovo on 9/24/2024.
//

#ifndef VKCELSHADINGRENDERER_EVENTHANDLE_H
#define VKCELSHADINGRENDERER_EVENTHANDLE_H
#include <barrier>

namespace hide {

    template<typename Derived, typename Event>
    class EventHandle_Common {
    public:
        auto up(const Event &e) { event = e; }
        auto va() const { return event.value(); }

    protected:
        std::optional<Event> event;
    };

    template<typename Event>
    class EventHandle_Immediate : public virtual EventHandle_Common<Event, Event> {
        using Handler = std::function<void(const Event &)>;
    public:
        auto sub(Handler h){ handlers.push_back(std::move(h)); }
        auto pub(const Event& e) {
            this->up(e);
            for (auto& handler : handlers){ handler(e); }
        }
    private:
        vot::vector<Handler> handlers;
    };

    template<typename Event>
    class EventHandle_Deferred : public virtual EventHandle_Common<Event, Event> {
        using Handler = std::function<void(const Event &)>;
    public:
        auto sub_queued(const size_t count, Handler h) -> void {
            participantCount = count;
            if (!syncBarrier) syncBarrier = std::make_unique<std::barrier<>>(participantCount);
            handlers.push_back(std::move(h));
        }
        auto pub_enqueue(const Event& e) -> void {
            this->up(e);
            queue.push(e);
        }

        auto dispatch() -> void {
            while (!queue.empty()) {
                auto& e = queue.front();

                syncBarrier->arrive_and_wait();

                if (bool expected = false; executed.compare_exchange_strong(expected, true)) {
                    for (auto &h: handlers) { h(e); }
                    queue.pop();
                }

                syncBarrier->arrive_and_wait();
                executed.store(false);
            }
        }
    private:
        std::atomic<bool> executed{false};
        vot::queue<Event> queue;
        std::vector<Handler> handlers;
        size_t participantCount = 0;
        std::unique_ptr<std::barrier<>> syncBarrier;
    };

    template<typename Event>
    class EventHandle_Latest : public virtual EventHandle_Common<Event, Event> { // UNUSE
        using Handler = std::function<void(const Event &)>;

    public:
        void sub_unique(Handler h) { handlers.push_back(std::move(h)); }
        void up(const Event &e) { this->event = e; }

        void process() {
            if (this->event) {
                for (auto &h: handlers) h(*this->event);
            }
        }

    private:
        std::vector<Handler> handlers;
    };



    template<typename Event>
    class EventHandleAll : public EventHandle_Immediate<Event>,
                            public EventHandle_Deferred<Event>,
                            public EventHandle_Latest<Event> {
    };

    template<typename Event>
    EventHandleAll<Event> *registerEvent() {
        static EventHandleAll<Event> e;
        return &e;
    }


} // hide

#endif //VKCELSHADINGRENDERER_EVENTHANDLE_H
