#pragma once

#include <functional>
#include <string>

class EventManager
{
public:
    template <typename... Args>
    static void Attach(const std::string& name, Args&&...args);

    static void Notify(const std::string& name);

private:
    static EventManager& GetInstance();

    EventManager();
    ~EventManager();

    std::unordered_map<std::string, std::vector<std::function<void ()>>> m_listener;

    void attach(const std::string& name, std::function<void ()> func);

    void notify(const std::string& name);
};

template <typename... Args>
inline void EventManager::Attach(const std::string& name, Args&&...args)
{
    if constexpr (sizeof...(Args) > 0)
        GetInstance().attach(name, std::bind(std::forward<Args>(args)...));
}



