#pragma once

#include <atomic>
#include <functional>
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
    template <typename ...Args>
    static void Add(Priority priority, Args &&...args);

    static void Update();

private:
    static MainThreadDispatcher &GetInstance();

    std::vector<std::function<void()>> m_events[static_cast<int>(Priority::COUNT)];

    std::vector<std::function<void()>> m_processingEvents;

    std::atomic<bool> m_hasEvent;
    
    std::mutex m_mutex;

    MainThreadDispatcher() = default;
    ~MainThreadDispatcher() = default;

    void update();

    void add(Priority priority, std::function<void()> func);
};

template <typename... Args>
inline void MainThreadDispatcher::Add(Priority priority, Args&&... args)
{
    return GetInstance().add(priority, std::move(std::bind(std::forward<Args>(args)...)));
}



