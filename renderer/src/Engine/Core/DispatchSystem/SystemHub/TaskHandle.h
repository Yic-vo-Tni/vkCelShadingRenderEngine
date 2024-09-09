//
// Created by lenovo on 9/8/2024.
//

#ifndef VKCELSHADINGRENDERER_TASKHANDLE_H
#define VKCELSHADINGRENDERER_TASKHANDLE_H

namespace Hide{

    class Task{
        using fn = std::function<void()>;
    public:
        explicit Task(std::atomic_bool& running) : running(running){
            thread = std::thread(&Task::loop, this);
        }
        ~Task(){
            running = false;
            cond.notify_all();

            if (thread.joinable())
                thread.join();
        }

        auto addTask(fn f) -> void {
            {
                std::lock_guard<std::mutex> lock(mutex);
                tasks.push(std::move(f));
            }
            cond.notify_one();
        }

        auto loop() -> void{
            fn t;
            while(running){
                std::unique_lock<std::mutex> lock(mutex);
                cond.wait(lock, [this]{ return !tasks.empty() || !running; });
                if (!running && tasks.empty()) break;
                t = std::move(tasks.front());
                tasks.pop();
            }
            t();
        }

    private:
        std::atomic_bool& running;
        std::thread thread;
        vot::queue<std::function<void()>> tasks;
        std::mutex mutex;
        std::condition_variable cond;
    };

    class TaskHandle{
    public:
        TaskHandle() = default;
        ~TaskHandle() {
        }

        auto eventLoop(){
            while(running){

            }
        }
        auto updateLoop() -> void{
            while(running){

            }
        }
        auto renderLoop() -> void{
            while(running){

            }
        }

    private:
        std::atomic<bool> running{true};
    };

}

#endif //VKCELSHADINGRENDERER_TASKHANDLE_H
