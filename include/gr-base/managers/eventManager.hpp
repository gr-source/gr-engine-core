#pragma once

#include "gr-base/core/ievent.hpp"

#include <functional>
#include <string>

class EventManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename... Args>
    static void Attach(const std::string& name, Args&&...args);

    static void Notify(const std::string& name, IEvent* event);

private:
    static EventManager* s_EventManager;

    EventManager();
    ~EventManager();

    std::unordered_map<std::string, std::vector<std::function<void (IEvent*)>>> m_listener;

    void attach(const std::string& name, std::function<void (IEvent*)> func);

    void notify(const std::string& name, IEvent* event);
};

template <typename... Args>
inline void EventManager::Attach(const std::string& name, Args&&...args)
{
    s_EventManager->attach(name, std::bind(std::forward<Args>(args)..., std::placeholders::_1));
}



