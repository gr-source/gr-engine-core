#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/clock.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/logger.hpp"
#include <ctime>
#include <vector>


MainThreadDispatcher* MainThreadDispatcher::s_MainThreadDispatcher = nullptr;

void MainThreadDispatcher::Initialize()
{
    if (s_MainThreadDispatcher == nullptr)
        s_MainThreadDispatcher = new MainThreadDispatcher();
}

void MainThreadDispatcher::Shutdown()
{
    if (s_MainThreadDispatcher != nullptr)
        delete s_MainThreadDispatcher;
    s_MainThreadDispatcher = nullptr;
}

// public
void MainThreadDispatcher::Process()
{
    return s_MainThreadDispatcher->process();
}

// private
MainThreadDispatcher::MainThreadDispatcher()
{
    logger::info("[ MainThreadDispatcher ] Initialize '%p'.", this);
}

MainThreadDispatcher::~MainThreadDispatcher()
{
    logger::info("[ MainThreadDispatcher ] Shutdown '%p'.", this);
}

void MainThreadDispatcher::process()
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
                Console::LogError("[ MainThread ] %s", e.what());
            }
        }

        m_processingEvents.clear();
    }
}

void MainThreadDispatcher::queue(std::function<void()> func, float seconds, Priority priority)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (seconds > 0.0f)
        m_timerHeap.insert({Clock::time + seconds, func});
    else
        m_events[static_cast<int>(priority)].push_back(std::move(func));

    m_hasEvent.store(true, std::memory_order_release);
}



