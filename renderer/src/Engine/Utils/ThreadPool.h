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

private:
    std::vector<std::thread> mWorkers;
    std::queue<std::function<void()>> mTasks;
    std::mutex mQueueMutex;
    std::condition_variable mCondition;
    bool mStop = false;
};


#endif //VKCELSHADINGRENDERER_THREADPOOL_H
