#include "gr-base/core/event.hpp"

std::list<std::function<void()>> Event::m_events;

std::mutex Event::m_mutex;

void Event::AddEvent(std::function<void()> func) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.push_back(func);
}

void Event::Update() {
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events.size();
}

