#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <mutex>

enum Priority
{
    HIGHT,
    NORMAL,
    LOW,
    COUNT
};

class MainThreadDispatcher
{
public:
    static void Initialize();

    static void Shutdown();

    static void Dispatch();

    template <typename ...Args>
    static void Queue(float seconds, Args&&...args);

    template <typename ...Args>
    static void Queue(Priority p, Args&&...args);

    template <typename ...Args>
    static void Queue(Args&&...args);

private:
    static MainThreadDispatcher* s_MainThreadDispatcher;

    static constexpr size_t TimerPriority = Priority::NORMAL;
    static constexpr size_t PriorityCount = Priority::COUNT;

    std::vector<std::function<void()>> priorityQueues[PriorityCount];

    std::multimap<float, std::function<void()>> timerQueue;

    std::vector<std::function<void()>> dispatchQueue;

    std::atomic<bool> hasPendingWork;
    
    std::mutex m_mutex;

    MainThreadDispatcher();
    ~MainThreadDispatcher();

    void dispatch();

    void queue(std::function<void()> func, float delaySeconds = 0.0f, Priority priority = Priority::NORMAL);
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



