#pragma once

#include <functional>
#include <algorithm>
#include <string>
#include <mutex>
#include <list>

#include <gr-math/gmath.hpp>

class Event {
public:
    template <typename ...Args>
    static void Add(Args &&...args);

    static void AddEvent(std::function<void()> func);

    static void ProcessEvent();

    static grm::size GetCount();

protected:
    static std::list<std::function<void()>> m_events;
    
    static std::mutex m_mutex;
};

template <typename... Args>
inline void Event::Add(Args&&... args) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.emplace_back(std::bind(std::forward<Args>(args)...));
}



