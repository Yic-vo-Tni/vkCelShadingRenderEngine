//
// Created by lenovo on 5/21/2024.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) {
    for(size_t i = 0; i < numThreads; i++){
        mWorkers.emplace_back([this]{
            while (true){
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mQueueMutex);
                    mCondition.wait(lock, [this]{
                        return mStop || !mTasks.empty();
                    });
                    if (mStop && mTasks.empty()) return ;
                    task = std::move(mTasks.front());
                    mTasks.pop();
                }
//                mActiveTasks ++;
                task();
//                mActiveTasks --;
            }
//            while (true){
//                std::function<void()> task;
//                {
//                    std::unique_lock<std::mutex> lock(mQueueMutex);
//                    mCondition.wait(lock, [this]{
//                        return mStop || !mTasks.empty();
//                    });
//                    if (mStop && mTasks.front()) return ;
//                    task = std::move(mTasks.front());
//                    mTasks.pop();
//                    ++mActiveTasks;
//                }
//                task();
//                {
//                    std::unique_lock<std::mutex> lock(mQueueMutex);
//                    --mActiveTasks;
//                    if (mActiveTasks == 0 && mTasks.empty()){
//                        mCompleted.notify_all();
//                    }
//                }
//            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(mQueueMutex);
        mStop = true;
    }
    mCondition.notify_all();
    for(std::thread& worker : mWorkers){
        worker.join();
    }
}