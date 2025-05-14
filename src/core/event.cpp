#include "gr-base/core/event.hpp"
#include "gr-base/core/console.hpp"

std::list<std::function<void()>> Event::m_events;

std::mutex Event::m_mutex;

void Event::AddEvent(std::function<void()> func)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_events.push_back(func);
}

void Event::Update()
{
    std::list<std::function<void()>> events;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        events = m_events;
        m_events.clear();
    }

    for (const auto& event : events)
    {
        try {
            event();
        } catch (const std::exception& e)
        {
            Console::LogError("[ Event ] Exception deceted: %s", e.what());
        }
    }
}

std::size_t Event::GetCount()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_events.size();
}

