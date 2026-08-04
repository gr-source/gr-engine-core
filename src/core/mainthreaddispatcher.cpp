#include "gr-base/core/mainthreaddispatcher.hpp"

#include "gr-base/core/clock.hpp"
#include "gr-base/core/console.hpp"
#include "gr-base/core/logger.hpp"

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
void MainThreadDispatcher::Dispatch()
{
    return s_MainThreadDispatcher->dispatch();
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

void MainThreadDispatcher::dispatch()
{
    if (!hasPendingWork.load(std::memory_order_acquire))
        return;

    while (true)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = timerQueue.begin();
            while (it != timerQueue.end() && it->first <= Clock::time)
            {
                priorityQueues[TimerPriority].push_back(std::move(it->second));
                it = timerQueue.erase(it);
            }

            for (size_t i=0;i<PriorityCount;i++)
            {
                if (priorityQueues[i].empty())
                    continue;

                dispatchQueue.insert(
                    dispatchQueue.end(),
                    std::make_move_iterator(priorityQueues[i].begin()),
                    std::make_move_iterator(priorityQueues[i].end())
                );
                
                priorityQueues[i].clear();
            }

            if (dispatchQueue.empty())
            {
                hasPendingWork.store(!timerQueue.empty(), std::memory_order_release);
                break;
            }
        }

        for (auto &event : dispatchQueue)
        {
            try {
                event();
            } catch (const std::exception& e)
            {
                Console::LogDefault("{FF0000}[ MainThread ] {FFFFFF}%s", e.what());
            }
        }

        dispatchQueue.clear();
    }
}

void MainThreadDispatcher::queue(std::function<void()> func, float delaySeconds, Priority priority)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (delaySeconds > 0.0f)
            timerQueue.insert({Clock::time + delaySeconds, func});
        else
            priorityQueues[priority].push_back(std::move(func));
    }

    hasPendingWork.store(true, std::memory_order_release);
}



