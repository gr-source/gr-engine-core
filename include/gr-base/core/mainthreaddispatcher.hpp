#pragma once

#include <functional>
#include <mutex>

class MainThreadDispatcher
{
public:
    template <typename ...Args>
    static void Add(Args &&...args);

    static void Update();

private:
    static MainThreadDispatcher &GetInstance();

    std::vector<std::function<void()>> m_events;
    
    std::mutex m_mutex;

    void update();

    void add(std::function<void()> func);
};

template <typename... Args>
inline void MainThreadDispatcher::Add(Args&&... args)
{
    return GetInstance().add(std::bind(std::forward<Args>(args)...));
}



