#pragma once

#include <condition_variable>
#include <functional>
#include <thread>
#include <utility>
#include <vector>
#include <mutex>
#include <list>

class Threadpool
{
public:
    template <typename ...Args>
    static void AddTask(Args &&...args);

    static bool IsMainThread();

    static std::size_t GetCount();

    static std::size_t GetUsage();

private:
    static Threadpool s_Threadpool;

    std::list<std::function<void()>> m_task;

    std::vector<std::thread> m_threads;

    std::condition_variable m_cv;

    static std::mutex m_mutex;

    bool m_bStop;

    std::uint8_t m_available;

    const std::thread::id m_mainThreadID;

    Threadpool(std::uint32_t maxThread);
    ~Threadpool();

    bool isMainThread() const;

    template <typename ...Args>
    void add(Args &&...args);

    void update();

    bool wait_for();
};

template <typename... Args>
inline void Threadpool::AddTask(Args &&...args)
{
    return s_Threadpool.add(std::forward<Args>(args)...);
}

// private
template <typename ...Args>
inline void Threadpool::add(Args &&...args)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task.emplace_back(std::bind(std::forward<Args>(args)...));
    }
    m_cv.notify_one();
}



