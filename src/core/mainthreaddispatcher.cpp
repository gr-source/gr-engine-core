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
    if (!m_hasEvent.load(std::memory_order_acquire))
        return;

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (uint8_t i=0;i<static_cast<uint8_t>(Priority::COUNT);i++)
        {
            if (m_events[i].empty())
                continue;

            m_processingEvents.insert(
                m_processingEvents.end(),
                std::make_move_iterator(m_events[i].begin()),
                std::make_move_iterator(m_events[i].end())
            );
            
            m_events[i].clear();
        }
        m_hasEvent.store(false, std::memory_order_release);
    }

    for (auto &event : m_processingEvents)
    {
        try {
            event();
        } catch (const std::exception& e)
        {
            Console::LogError("[ MainThreadDispatcher ] Exception deceted: %s", e.what());
        }
    }

    m_processingEvents.clear();
}

void MainThreadDispatcher::add(Priority priority, std::function<void()> func)
{
    if (func == nullptr)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_events[static_cast<int>(priority)].push_back(std::move(func));

    m_hasEvent.store(true, std::memory_order_release);
}



