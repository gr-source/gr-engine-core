#pragma once

#include <functional>
#include <mutex>
#include <list>

// MainThreadDispatcher
class Event {
public:
    template <typename ...Args>
    static void Add(Args &&...args);

    static void AddEvent(std::function<void()> func);

    static void Update();

    static std::size_t GetCount();

protected:
    static std::list<std::function<void()>> m_events;
    
    static std::mutex m_mutex;
};

template <typename... Args>
inline void Event::Add(Args&&... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace_back(std::bind(std::forward<Args>(args)...));
}



