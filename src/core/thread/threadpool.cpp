#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/thread/default.hpp"

#include <mutex>
#include <thread>

// static
bool Threadpool::IsMainThread()
{
    return GetInstance().isMainThread();
}

std::size_t Threadpool::GetThreadCount()
{
    return GetInstance().get_thread_count();
}

std::size_t Threadpool::GetCount()
{
    return GetInstance().get_count();
}

// private
Threadpool &Threadpool::GetInstance()
{
    static Threadpool instance(std::thread::hardware_concurrency());
    return instance;
}

Threadpool::Threadpool(std::uint32_t maxThread) : m_bStop(false), m_available(0), m_mainThreadID(std::this_thread::get_id())
{
    Console::LogInfo("[ Threadpool ] Initialize");

    for (std::uint32_t i=0;i<maxThread;i++)
    {
        m_threads.emplace_back(&Threadpool::update, this);
    }
}

Threadpool::~Threadpool()
{
    Console::LogInfo("[ Threadpool ] Shutdown");
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [&] { return m_available == 0 && m_task.empty(); });
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bStop = true;
    }

    m_cv.notify_all();

    for (auto &thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    Console::LogInfo("[ Threadpool ] Threadpool stoped.");
}

bool Threadpool::isMainThread() const
{
    return std::this_thread::get_id() == m_mainThreadID;
}

void Threadpool::update()
{
    Console::LogInfo("[ Threadpool ] thread %ld, in background...", std::this_thread::get_id());

    while (true)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, std::bind(&Threadpool::wait_for, this));

        if (m_bStop)
            break;


        if (m_available >= 3)
        {
            bool haspriority = false;
            for (auto &&[priority, task] : m_task)
            {
                if (priority == TaskPriority::HIGH)
                {
                    haspriority = true;
                    break;
                }
            }

            if (!haspriority)
            {
                continue;
            }
        }

        {
            m_available++;
            m_cv.notify_all();
        }

        auto temp = std::move(m_task);
        lock.unlock();

        temp.sort([](auto &lhs, auto &rhs) { return std::get<0>(lhs) > std::get<0>(rhs); });

        try
        {
            for (auto &&[priority, task] : temp)
                task();
        } catch (const std::exception &e)
        {
            Console::LogError("[ Threadpool ] Exception detected: %s", e.what());
        }

        lock.lock();
        {
            m_available--;
            m_cv.notify_all();
        }
    }
}

bool Threadpool::wait_for()
{
    return m_bStop || !m_task.empty();
}

std::size_t Threadpool::get_thread_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_threads.size();
}

std::size_t Threadpool::get_count()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_available;
}

