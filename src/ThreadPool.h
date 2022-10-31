#pragma once

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace SoftRenderer
{
    class ThreadPool
    {
    public:
        explicit ThreadPool(const size_t threadCnt = std::thread::hardware_concurrency())
            : mThreadCnt(threadCnt)
            , mThreads(new std::thread[mThreadCnt])
            {
                createThreads();
            }

            ~ThreadPool() 
            {
                waitTasksFinish();
                joinThreads();

                mIsRunning = false;
            }

            template<typename F>
            void addTask(const F & task) 
            {
                mTasksCnt++;
                {
                    const std::lock_guard<std::mutex> lock(mMutex);
                    mTasks.push(std::function<void(size_t)>(task));
                }
            }

            template<typename F, typename... A>
            void addTask(const F & task, const A &...args) 
            {
                addTask([task, args...]{ task(args...); });
            }

            void waitTasksFinish() const 
            {
                while (true) 
                {
                    if (!mIsPaused)
                    {
                        if (mTasksCnt == 0)
                            break;
                    }
                    else 
                    {
                        if (getRunningTaskCount() == 0)
                            break;
                    }

                    std::this_thread::yield();
                }
            }

            

    private:
        void createThreads() 
        {
            for (size_t i = 0; i < mThreadCnt; i++) 
            {
                mThreads[i] = std::thread(&ThreadPool::taskWorker, this, i);
            }
        }

        void joinThreads() 
        {
            for (size_t i = 0; i < mThreadCnt; i++) 
            {
                mThreads[i].join();
            }
        }

        size_t getWaitingTaskCount() const 
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            return mTasks.size();
        }

        size_t getRunningTaskCount() const 
        {
            return mTasksCnt - getWaitingTaskCount();
        }

        bool popTask(std::function<void(size_t)>& task) 
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            if (mTasks.empty())
                return false;

            task = std::move(mTasks.front());
            mTasks.pop();
            return true;
        }

        void taskWorker(size_t thread_id) 
        {
            while (mIsRunning) 
            {
                std::function<void(size_t)> task;
                if (!mIsPaused && popTask(task))
                {
                    task(thread_id);
                    mTasksCnt--;
                }
                else 
                {
                    std::this_thread::yield();
                }
            }
        }

    private:
        mutable std::mutex mMutex = {};

        std::atomic<bool> mIsRunning = true;
        std::atomic<bool> mIsPaused = false;

        size_t mThreadCnt;
        std::unique_ptr<std::thread[]> mThreads;

        std::queue<std::function<void(size_t)>> mTasks = {};
        std::atomic<size_t> mTasksCnt = 0;
    };
}