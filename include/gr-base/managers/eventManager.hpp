#pragma once

#include <functional>
#include <string>

class EventManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename... Args>
    static void Attach(const std::string& name, Args&&...args);

    static void Notify(const std::string& name);

private:
    static EventManager* s_EventManager;

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
        s_EventManager->attach(name, std::bind(std::forward<Args>(args)...));
}



