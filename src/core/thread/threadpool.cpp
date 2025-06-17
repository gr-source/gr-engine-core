#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/console.hpp"

#include <mutex>
#include <thread>

std::mutex Threadpool::m_mutex;

// static
bool Threadpool::IsMainThread()
{
    return GetInstance().isMainThread();
}

std::size_t Threadpool::GetCount()
{
    return GetInstance().m_threads.size();
}

std::size_t Threadpool::GetUsage()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return GetInstance().m_available;
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
        {
            break;
        }

        {
            m_available++;
            m_cv.notify_all();
        }

        auto temp = std::move(m_task);
        lock.unlock();

        try
        {
            for (auto &task : temp)
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


