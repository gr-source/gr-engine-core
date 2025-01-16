#include "core/event.hpp"

std::list<std::function<void()>> Event::m_events;

std::mutex Event::m_mutex;

void Event::AddEvent(std::function<void()> func) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.push_back(func);
}

void Event::ProcessEvent() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (!m_events.empty()) {
        auto event = m_events.front();
        m_events.pop_front();
        lock.unlock();

        event();

        lock.lock();
    }
}

grm::size Event::GetCount() {
    grm::size count = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        count = m_events.size();
    }
    return count;
}

