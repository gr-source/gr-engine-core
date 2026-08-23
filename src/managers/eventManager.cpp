#include "gr-base/managers/eventManager.hpp"

#include "gr-base/core/logger.hpp"
#include "gr-base/core/mainthreaddispatcher.hpp"
#include <atomic>

EventManager* EventManager::s_EventManager = nullptr;

void EventManager::Initialize()
{
    if (s_EventManager == nullptr)
        s_EventManager = new EventManager();
}

void EventManager::Shutdown()
{
    if (s_EventManager != nullptr)
        delete s_EventManager;
    s_EventManager = nullptr;
}

// private
EventID EventManager::GenEventID()
{
    static std::atomic<EventID> nextID = 0;
    return nextID++;
}

EventManager::EventManager()
{
    logger::info("[ EventManager ] Initialized '%p'.", this);
}

EventManager::~EventManager()
{
    logger::info("[ EventManager ] Shutdown '%p'.", this);
}

void EventManager::subscribe(EventID id, std::function<void (IEvent*)> func)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (id >= m_listener.size())
            m_listener.resize(id + 1);

        m_listener[id].push_back(std::move(func));
    }

    logger::debug("[ EventManager ] Attached event '%d'.", id);
}

void EventManager::unsubscribe(EventID id)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (id >= m_listener.size())
            m_listener.resize(id + 1);

        m_listener[id].clear();
    }

    logger::debug("[ EventManager ] Detached event '%d'.", id);
}

void EventManager::notify(EventID id, IEvent* event)
{
    std::vector<std::function<void (IEvent*)>> execute;

    {
        std::scoped_lock<std::mutex> lock(m_mutex);
        if (id >= m_listener.size())
            return;

        execute = m_listener[id];
    }

    for (auto& func : execute)
        func(event);
}

void EventManager::notifyAsync(EventID id, std::shared_ptr<IEvent> event)
{
    MainThreadDispatcher::Queue([this, id, event]{
            notify(id, event.get());
        });
}


