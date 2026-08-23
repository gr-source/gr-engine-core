#pragma once

#include "gr-base/core/ievent.hpp"

#include <functional>
#include <cstdint>
#include <memory>
#include <mutex>

typedef uint32_t EventID;

class EventManager
{
public:
    static void Initialize();

    static void Shutdown();

    template <typename T, typename Callback, typename... Args>
    static void Subscribe(Callback&& callback, Args&&...args);

    template <typename T>
    static void Unsubscribe();

    template <typename T>
    static void Emit(T&& event);

    template <typename T>
    static void EmitAsync(T&& event);

private:
    static EventManager* s_EventManager;

    static EventID GenEventID();

    template <typename T>
    static EventID GetEventIDByType();

    EventManager();
    ~EventManager();

    std::vector<std::vector<std::function<void (IEvent*)>>> m_listener;

    std::mutex m_mutex;

    void subscribe(EventID id, std::function<void (IEvent*)> func);

    void unsubscribe(EventID id);

    void notify(EventID id, IEvent* event);

    void notifyAsync(EventID id, std::shared_ptr<IEvent> event);
};

template <typename T, typename Callback, typename... Args>
inline void EventManager::Subscribe(Callback&& callback, Args&&...args)
{
    static_assert(
        std::is_invocable_v<Callback, Args..., T&>,
        "Callback must be invocable with (..., T&)"
    );

    auto arguments = std::make_tuple(std::forward<Args>(args)...);

    s_EventManager->subscribe(
            GetEventIDByType<T>(), [callback = std::forward<Callback>(callback),
            arguments = std::move(arguments)](IEvent* event) mutable
            {
                std::apply(
                    [&](auto&&... unpacked)
                    {
                        std::invoke(callback, unpacked..., *static_cast<T*>(event));
                    },
                    arguments
                );
            });
}

template <typename T>
inline void EventManager::Unsubscribe()
{
    s_EventManager->unsubscribe(GetEventIDByType<T>());
}

template <typename T>
inline void EventManager::Emit(T&& event)
{
    s_EventManager->notify(GetEventIDByType<T>(), &event);
}

template <typename T>
inline void EventManager::EmitAsync(T&& event)
{
    auto copy = std::make_shared<T>(
        std::forward<T>(event)
    );

    s_EventManager->notifyAsync(GetEventIDByType<T>(), copy);
}

template <typename T>
inline EventID EventManager::GetEventIDByType()
{
    static EventID id = GenEventID();
    return id;
}


