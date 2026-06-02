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
    static void AddDelayed(float seconds, Args &&...args);

    template <typename ...Args>
    static void Add(Priority priority, Args &&...args);

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

    void addDelayed(float seconds, std::function<void()> func);

    void add(Priority priority, std::function<void()> func);
};

template <typename... Args>
inline void MainThreadDispatcher::AddDelayed(float seconds, Args&&... args)
{
    return s_MainThreadDispatcher->addDelayed(seconds, std::move(std::bind(std::forward<Args>(args)...)));
}

template <typename... Args>
inline void MainThreadDispatcher::Add(Priority priority, Args&&... args)
{
    return s_MainThreadDispatcher->add(priority, std::move(std::bind(std::forward<Args>(args)...)));
}



