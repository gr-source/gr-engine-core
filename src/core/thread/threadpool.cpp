#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/thread/default.hpp"

#include "gr-base/core/logger.hpp"

#include <thread>
#include <mutex>

Threadpool* Threadpool::s_Threadpool = nullptr;

// static
void Threadpool::Initialize()
{
    if (s_Threadpool == nullptr)
        s_Threadpool = new Threadpool(std::thread::hardware_concurrency());
}

void Threadpool::Shutdown()
{
    if (s_Threadpool != nullptr)
        delete s_Threadpool;

    s_Threadpool = nullptr;
}

bool Threadpool::IsMainThread()
{
    return s_Threadpool->isMainThread();
}

std::size_t Threadpool::GetThreadCount()
{
    return s_Threadpool->get_thread_count();
}

size_t Threadpool::GetJobCount()
{
    return s_Threadpool->pendingJobs.load(std::memory_order_acquire);
}

std::size_t Threadpool::GetCount()
{
    return s_Threadpool->get_count();
}

// private
Threadpool::Threadpool(uint32_t maxThread) :
    threadUsedCount(0),
    m_mainThreadID(std::this_thread::get_id())
{
    logger::info("[ Threadpool ] Initialize '%p'.", this);

    for (uint32_t i=0;i<maxThread;i++)
        m_threads.emplace_back(&Threadpool::workerThread, this);

    logger::info("[ Threadpool] Started with %u worker threads.", maxThread);
}

Threadpool::~Threadpool()
{
    logger::info("[ Threadpool ] Stopping %zu worker threads.", m_threads.size());

    stopRequested.store(true, std::memory_order_release);

    m_cv.notify_all();

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(
                lock,
                [&]{
                    return activeWorkers.load(std::memory_order_acquire) == 0;
                }
            );
    }

    for (auto &thread : m_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    logger::info("[ Threadpool ] Shutdown '%p'.", this);
}

bool Threadpool::isMainThread() const
{
    return std::this_thread::get_id() == m_mainThreadID;
}

void Threadpool::add(Priority priority, std::function<void()> func)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        jobQueues[priority].push_back(std::move(func));
    }

    pendingJobs.fetch_add(1, std::memory_order_release);

    m_cv.notify_all();
}

void Threadpool::workerThread()
{
    activeWorkers.fetch_add(1, std::memory_order_release);

    while (true)
    {
        std::function<void()> task = nullptr;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(
                    lock,
                    [this]
                    {
                        return
                            stopRequested.load(std::memory_order_acquire) ||
                            pendingJobs.load(std::memory_order_acquire) != 0;
                    }
                );

            if (stopRequested.load(std::memory_order_acquire))
                break;

            for (size_t i = 0; i <Priority::COUNT; i++)
            {
                if (jobQueues[i].empty())
                    continue;

                task = std::move(jobQueues[i].front());
                jobQueues[i].pop_front();

                pendingJobs.fetch_sub(1, std::memory_order_release);

                break;
            }
        }

        if (task)
        {
            threadUsedCount.fetch_add(1, std::memory_order_release);

            try
            {
                task();
            } catch (const std::exception &e)
            {
                Console::LogDefault("{FF0000}[ Threadpool ] {FFFFFF}%s", e.what());
            }

            threadUsedCount.fetch_sub(1, std::memory_order_release);
        }
    }

    activeWorkers.fetch_sub(1, std::memory_order_release);

    m_cv.notify_all();
}

size_t Threadpool::get_thread_count()
{
    return activeWorkers.load(std::memory_order_acquire);
}

size_t Threadpool::get_count()
{
    return threadUsedCount.load(std::memory_order_acquire);
}

