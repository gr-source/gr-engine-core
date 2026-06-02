#pragma once

#include <condition_variable>
#include <functional>
#include <list>
#include <thread>
#include <utility>
#include <vector>
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
    static void AddTask(TaskPriority priority, Args &&...args);

    static bool IsMainThread();

    static size_t GetThreadCount();

    static size_t GetCount();

private:
    static Threadpool* s_Threadpool;

    std::list<std::function<void()>> m_task[static_cast<uint8_t>(TaskPriority::COUNT)];

    std::vector<std::thread> m_threads;

    std::condition_variable m_cv;

    std::mutex m_mutex;

    bool m_bStop;

    bool m_hasTask;

    std::uint8_t m_available;

    const std::thread::id m_mainThreadID;

    Threadpool(uint32_t maxThread);
    ~Threadpool();

    bool isMainThread() const;

    void add(TaskPriority priority, std::function<void()> func);

    void update();


    std::size_t get_thread_count();

    std::size_t get_count();
};

template <typename... Args>
inline void Threadpool::AddTask(Args &&...args)
{
    return s_Threadpool->add(TaskPriority::LOW, std::move(std::bind(std::forward<Args>(args)...)));
}

template <typename... Args>
inline void Threadpool::AddTask(TaskPriority priority, Args &&...args)
{
    return s_Threadpool->add(priority, std::move(std::bind(std::forward<Args>(args)...)));
}




