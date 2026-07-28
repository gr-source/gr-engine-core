#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>

enum class Priority : uint8_t
{
    HIGHT = 0,
    NORMAL = 1,
    LOW = 2,
    COUNT
};

class MainThreadDispatcher
{
public:
    static void Initialize();

    static void Shutdown();

    static void Process();

    template <typename ...Args>
    static void Queue(float seconds, Args&&...args);

    template <typename ...Args>
    static void Queue(Priority p, Args&&...args);

    template <typename ...Args>
    static void Queue(Args&&...args);

private:
    static MainThreadDispatcher* s_MainThreadDispatcher;

    std::vector<std::function<void()>> m_events[static_cast<int>(Priority::COUNT)];

    std::vector<std::function<void()>> m_processingEvents;

    std::multimap<float, std::function<void()>> m_timerHeap;

    std::atomic<bool> m_hasEvent;
    
    std::mutex m_mutex;

    MainThreadDispatcher();
    ~MainThreadDispatcher();

    void process();

    void queue(std::function<void()> func, float seconds = 0.0f, Priority priority = Priority::NORMAL);
};

template <typename... Args>
inline void MainThreadDispatcher::Queue(float seconds, Args&&... args)
{
    return s_MainThreadDispatcher->queue(
            std::move(std::bind(std::forward<Args>(args)...)),
            seconds
        );
}

template <typename... Args>
inline void MainThreadDispatcher::Queue(Priority p, Args&&... args)
{
    return s_MainThreadDispatcher->queue(
            std::move(std::bind(std::forward<Args>(args)...)),
            0.0f,
            p
        );
}

template <typename... Args>
inline void MainThreadDispatcher::Queue(Args&&... args)
{
    return s_MainThreadDispatcher->queue(std::move(std::bind(std::forward<Args>(args)...)));
}



