#pragma once

#include <condition_variable>
#include <functional>
#include <list>
#include <thread>
#include <utility>
#include <vector>
#include <atomic>
#include <mutex>

#include "default.hpp"

class Threadpool
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename ...Args>
    static void AddTask(Args &&...args);

    template <typename ...Args>
    static void AddTask(Priority priority, Args &&...args);

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

    void add(Priority priority, std::function<void()> func);

    void workerThread();

    size_t get_thread_count();

    size_t get_count();
};

template <typename... Args>
inline void Threadpool::AddTask(Args &&...args)
{
    return s_Threadpool->add(Priority::NORMAL, std::move(std::bind(std::forward<Args>(args)...)));
}

template <typename... Args>
inline void Threadpool::AddTask(Priority priority, Args &&...args)
{
    return s_Threadpool->add(priority, std::move(std::bind(std::forward<Args>(args)...)));
}




