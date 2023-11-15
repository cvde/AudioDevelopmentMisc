#pragma once

#include <atomic>
#include <atomic_queue/atomic_queue.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace edsp
{

template <int MAX_QUEUE_SIZE>
class ThreadPool
{
public:
    ThreadPool()
    {
        size_t numThreads = std::thread::hardware_concurrency() > 0 ? std::thread::hardware_concurrency() : 1;
        mThreads.reserve(numThreads);
        for (size_t i = 0; i < numThreads; ++i)
        {
            mThreads.emplace_back([this]
                                  { threadFunction(); });
        }
    }

    ~ThreadPool()
    {
        mStopThreads = true;
        mCondition.notify_all();
        for (auto& thread : mThreads)
            thread.join();
    }

    ThreadPool(const ThreadPool& other) = delete;
    ThreadPool& operator=(const ThreadPool& other) = delete;

    template <typename F, typename... A>
    bool enqueue(F&& task, A&&... args) noexcept
    {
        if (!mTasks.try_push(std::bind(std::forward<F>(task), std::forward<A>(args)...)))
            return false;

        mCondition.notify_one();
        return true;
    }

private:
    void threadFunction()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mMutex);
                mCondition.wait(lock, [this]
                                { return !mTasks.was_empty() || mStopThreads; });
            }
            if (mStopThreads)
                return;
            if (mTasks.try_pop(task))
                task();
        }
    }

    std::vector<std::thread> mThreads;
    std::atomic<bool> mStopThreads = false;
    atomic_queue::AtomicQueue2<std::function<void()>, MAX_QUEUE_SIZE> mTasks;
    std::condition_variable mCondition;
    std::mutex mMutex;
};

} // namespace edsp
