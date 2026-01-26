#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/clock.hpp"
#include "gr-base/core/console.hpp"
#include <ctime>
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

    while (true)
    {

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_timerHeap.begin();
            while (it != m_timerHeap.end() && it->first <= Clock::time)
            {
                m_events[static_cast<int>(Priority::NORMAL)].push_back(std::move(it->second));
                it = m_timerHeap.erase(it);
            }

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

            if (m_processingEvents.empty()) {
                m_hasEvent.store(!m_timerHeap.empty(), std::memory_order_release);
                break;
            }
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
}

void MainThreadDispatcher::addDelayed(float seconds, std::function<void()> func)
{
    if (func == nullptr)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_timerHeap.insert({Clock::time + seconds, func});

    m_hasEvent.store(true, std::memory_order_release);
}

void MainThreadDispatcher::add(Priority priority, std::function<void()> func)
{
    if (func == nullptr)
        return;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_events[static_cast<int>(priority)].push_back(std::move(func));

    m_hasEvent.store(true, std::memory_order_release);
}



