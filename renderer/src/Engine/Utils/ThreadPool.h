//
// Created by lenovo on 5/21/2024.
//

#ifndef VKCELSHADINGRENDERER_THREADPOOL_H
#define VKCELSHADINGRENDERER_THREADPOOL_H


class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    template<typename F>
    void enqueue(F&& f){
        {
            std::unique_lock<std::mutex> lock(mQueueMutex);
            mTasks.emplace(std::forward<F>(f));
        }
        mCondition.notify_one();
    }

    auto waitAll() -> void{
//        while (mActiveTasks > 0){
//            std::this_thread::sleep_for(std::chrono::milliseconds(10));
//        }
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mCompleted.wait(lock, [this]{ return mTasks.empty() && mActiveTasks == 0;});
    }

private:
    std::vector<std::thread> mWorkers;
    std::queue<std::function<void()>> mTasks;
    std::mutex mQueueMutex;
    std::condition_variable mCondition;
    std::condition_variable mCompleted;
    bool mStop = false;
    std::atomic<int> mActiveTasks{0};
};


#endif //VKCELSHADINGRENDERER_THREADPOOL_H
