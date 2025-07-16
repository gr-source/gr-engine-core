#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>
#include <list>

#include "default.hpp"

class Threadpool
{
public:
    template <typename ...Args>
    static void AddTask(Args &&...args);

    template <typename ...Args>
    static void AddTask(TaskPriority priority, Args &&...args);

    static bool IsMainThread();

    static std::size_t GetThreadCount();

    static std::size_t GetCount();

private:
    static Threadpool &GetInstance();

    std::list<std::tuple<TaskPriority, std::function<void()>>> m_task;

    std::vector<std::thread> m_threads;

    std::condition_variable m_cv;

    std::mutex m_mutex;

    bool m_bStop;

    std::uint8_t m_available;

    const std::thread::id m_mainThreadID;

    Threadpool(std::uint32_t maxThread);
    ~Threadpool();

    bool isMainThread() const;

    template <typename ...Args>
    void add(TaskPriority priority, Args &&...args);

    void update();

    bool wait_for();

    std::size_t get_thread_count();

    std::size_t get_count();
};

template <typename... Args>
inline void Threadpool::AddTask(Args &&...args)
{
    return GetInstance().add(TaskPriority::LOW, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Threadpool::AddTask(TaskPriority priority, Args &&...args)
{
    return GetInstance().add(priority, std::forward<Args>(args)...);
}

// private
template <typename ...Args>
inline void Threadpool::add(TaskPriority priority, Args &&...args)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task.emplace_back(priority, std::bind(std::forward<Args>(args)...));
    }

    m_cv.notify_all();
}



