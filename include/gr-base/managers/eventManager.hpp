#pragma once

#include "gr-base/core/ievent.hpp"

#include <functional>
#include <cstdint>

typedef uint32_t EventID;

class EventManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename Callback, typename... Args>
    static void Attach(Callback&& callback, Args&&...args);

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

template <typename T, typename Callback, typename... Args>
inline void EventManager::Attach(Callback&& callback, Args&&...args)
{
    static_assert(
        std::is_invocable_v<Callback, Args..., T*>,
        "Callback must be invocable with (..., T*)"
    );

    auto arguments = std::make_tuple(std::forward<Args>(args)...);

    std::function<void(IEvent*)> function =
        [callback = std::forward<Callback>(callback),
         arguments = std::move(arguments)](IEvent* event) mutable
        {
            std::apply(
                [&](auto&&... unpacked)
                {
                    std::invoke(callback, unpacked..., static_cast<T*>(event));
                },
                arguments
            );
        };

    s_EventManager->attach(
        GetEventIDByType<T>(), function);
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


