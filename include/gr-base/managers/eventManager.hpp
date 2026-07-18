#pragma once

#include "gr-base/core/ievent.hpp"

#include <functional>
#include <cstdint>
#include <string>

typedef uint32_t EventID;

class EventManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename... Args>
    static void Attach(Args&&...args);

    template <typename T>
    static void Notify(T* event);

private:
    static EventManager* s_EventManager;

    static EventID GenEventID();

    template <typename T>
    static EventID GetEventIDByType();

    EventManager();
    ~EventManager();

    std::vector<std::vector<std::function<void (IEvent*)>>> m_listener;

    void attach(EventID id, std::function<void (IEvent*)> func);

    void notify(EventID id, IEvent* event);
};

template <typename T, typename... Args>
inline void EventManager::Attach(Args&&...args)
{
    s_EventManager->attach(GetEventIDByType<T>(), std::bind(std::forward<Args>(args)..., std::placeholders::_1));
}

template <typename T>
inline void EventManager::Notify(T* event)
{
    s_EventManager->notify(GetEventIDByType<T>(), event);
}

template <typename T>
inline EventID EventManager::GetEventIDByType()
{
    static EventID id = GenEventID();
    return id;
}


