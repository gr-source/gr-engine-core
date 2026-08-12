#pragma once

#include "gr-base/core/thread/default.hpp"

#include <condition_variable>
#include <functional>
#include <list>
#include <thread>
#include <utility>
#include <vector>
#include <atomic>
#include <mutex>

class Threadpool
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename ...Args>
    static void Submit(Args &&...args);

    template <typename ...Args>
    static void Submit(Priority priority, Args &&...args);

    static bool IsMainThread();

    static size_t GetThreadCount();

    static size_t GetJobCount();

    static size_t GetCount();

private:
    static Threadpool* s_Threadpool;

    std::list<std::function<void()>> jobQueues[Priority::COUNT];

    std::vector<std::thread> m_threads;

    std::condition_variable m_cv;

    std::mutex m_mutex;

    std::atomic<bool> stopRequested = false;

    // bool m_hasTask;

    std::atomic<uint32_t> pendingJobs = 0;

    std::atomic<uint32_t> threadUsedCount = 0;
    std::atomic<uint32_t> activeWorkers = 0;

    const std::thread::id m_mainThreadID;

    Threadpool(uint32_t maxThread);
    ~Threadpool();

    bool isMainThread() const;

    void submit(Priority priority, std::function<void()> func);

    void workerThread();

    size_t get_thread_count();

    size_t get_count();
};

template <typename... Args>
inline void Threadpool::Submit(Args &&...args)
{
    return s_Threadpool->submit(Priority::NORMAL, std::move(std::bind(std::forward<Args>(args)...)));
}

template <typename... Args>
inline void Threadpool::Submit(Priority priority, Args &&...args)
{
    return s_Threadpool->submit(priority, std::move(std::bind(std::forward<Args>(args)...)));
}




