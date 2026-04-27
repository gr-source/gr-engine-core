#include "gr-base/core/thread/threadpool.hpp"

#include "gr-base/core/console.hpp"
#include "gr-base/core/logger.hpp"
#include "gr-base/core/thread/default.hpp"

#include <thread>
#include <mutex>

Threadpool* Threadpool::s_Threadpool = nullptr;

// static
void Threadpool::Initialize()
{
    if (s_Threadpool == nullptr)
        s_Threadpool = new Threadpool(std::thread::hardware_concurrency());

    logger::info("[Threadpool] Initialize");
}

void Threadpool::Shutdown()
{
    if (s_Threadpool != nullptr)
        delete s_Threadpool;

    s_Threadpool = nullptr;

    logger::info("[Threadpool] Shutdown");
}

bool Threadpool::IsMainThread()
{
    return s_Threadpool->isMainThread();
}

std::size_t Threadpool::GetThreadCount()
{
    return s_Threadpool->get_thread_count();
}

std::size_t Threadpool::GetCount()
{
    return s_Threadpool->get_count();
}

// private
Threadpool::Threadpool(std::uint32_t maxThread) : m_bStop(false), m_hasTask(false), m_available(0), m_mainThreadID(std::this_thread::get_id())
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
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bStop = true;
    }
    m_cv.notify_all();

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [&] { return !m_available || !m_hasTask; });
    }

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

void Threadpool::add(TaskPriority priority, std::function<void()> func)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task[static_cast<uint8_t>(priority)].push_back(std::move(func));
        m_hasTask = true;
    }

    m_cv.notify_all();
}

void Threadpool::update()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this]{ return m_bStop || m_hasTask; });

            if (m_bStop && !m_hasTask)
                break;

            bool found = false;
            for (uint8_t i = 0; i < static_cast<uint8_t>(TaskPriority::COUNT); i++)
            {
                if (!m_task[i].empty())
                {
                    task = std::move(m_task[i].front());
                    m_task[i].pop_front();
                    found = true;
                    
                    bool stillHasWork = false;
                    for(uint8_t j = 0; j < static_cast<uint8_t>(TaskPriority::COUNT); j++)
                    {
                        if(!m_task[j].empty())
                        {
                            stillHasWork = true;
                            break;
                        }
                    }
                    m_hasTask = stillHasWork;
                    break; 
                }
            }

            if (!found)
            {
                m_hasTask = false;
                continue;
            }

            m_available++; 
        }

        if (task)
        {
            try
            {
                task();
            } catch (const std::exception &e)
            {
                Console::LogError("[ Threadpool ] Exception: %s", e.what());
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_available--;
            m_cv.notify_all(); 
        }
    }
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

