#include "core/threadpool.hpp"

std::list<std::function<void()>> Threadpool::m_task;

std::vector<std::thread> Threadpool::m_threads;

std::condition_variable Threadpool::m_cv;

std::size_t Threadpool::m_available = 0;

std::mutex Threadpool::m_mutex;

bool Threadpool::m_bStop = false;

void Threadpool::Initialize(std::uint32_t maxThread) {
    for (std::uint32_t i=0;i<maxThread;i++) {
        m_threads.emplace_back(&Threadpool::Update);
    }
}

void Threadpool::Shutdown() {
    Stop();

    for (auto &thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Threadpool::Stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_bStop = true;
    }
    m_cv.notify_all();
}

std::size_t Threadpool::GetThreadCount() {
    return m_threads.size();
}

std::size_t Threadpool::GetThreadAvailable() {
    return m_available;
}

void Threadpool::CreateInternalTask(std::function<void()> func) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_task.emplace_back(func);
    }
    m_cv.notify_all();
}

void Threadpool::Update() {
    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [] {
            return m_bStop || !m_task.empty();
        });

        if (m_bStop) {
            break;
        }
        m_available++;

        auto task = m_task.front();
        m_task.pop_front();
        lock.unlock();

        if (task)
            task();

        lock.lock();
        m_available--;
    }
}
