#include "gr-base/managers/eventManager.hpp"

#include "gr-base/core/logger.hpp"

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
    static EventID nextID = 0;
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

void EventManager::attach(EventID id, std::function<void (IEvent*)> func)
{
    if (id >= m_listener.size())
        m_listener.resize(id + 1);

    m_listener[id].push_back(func);

    logger::debug("[ EventManager ] Attached event '%d'.", id);
}

void EventManager::notify(EventID id, IEvent* event)
{
    if (id >= m_listener.size())
        return;

    for (auto& func : m_listener[id])
        func(event);

    logger::debug("[ EventManager ] Call event '%d'.", id);
}


