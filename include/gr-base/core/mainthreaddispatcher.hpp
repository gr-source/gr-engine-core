#pragma once

#include <functional>
#include <mutex>
#include <list>

class MainThreadDispatcher
{
public:
    template <typename ...Args>
    static void Add(Args &&...args);

    static void Update();

private:
    static MainThreadDispatcher &GetInstance();

    std::list<std::function<void()>> m_events;
    
    std::mutex m_mutex;

    template <typename ...Args>
    void add(Args &&...args);

    void update();
};

template <typename... Args>
inline void MainThreadDispatcher::Add(Args&&... args)
{
    return GetInstance().add(std::forward<Args>(args)...);
}

template <typename... Args>
inline void MainThreadDispatcher::add(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace_back(std::bind(std::forward<Args>(args)...));
}



