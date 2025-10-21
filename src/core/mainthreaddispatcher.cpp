#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/console.hpp"
#include <vector>

// public
void MainThreadDispatcher::Update()
{
    return GetInstance().update();
}

// private
MainThreadDispatcher &MainThreadDispatcher::GetInstance()
{
    static MainThreadDispatcher instance;
    return instance;
}

void MainThreadDispatcher::update()
{
    std::vector<std::function<void()>> events;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        events = std::move(m_events);
    }

    for (auto &event : events)
    {
        try {
            event();
        } catch (const std::exception& e)
        {
            Console::LogError("[ MainThreadDispatcher ] Exception deceted: %s", e.what());
        }
    }
}

