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

void EventManager::Notify(const std::string &name)
{
    s_EventManager->notify(name);
}

// private
EventManager::EventManager()
{
    logger::info("[ EventManager ] Initialized '%p'.", this);
}

EventManager::~EventManager()
{
    logger::info("[ EventManager ] Shutdown '%p'.", this);
}

void EventManager::attach(const std::string &name, std::function<void ()> func)
{
    m_listener[name].push_back(func);

    logger::debug("[ EventManager ] Attached function: %s", name.c_str());
}

void EventManager::notify(const std::string &name)
{
    for (auto& func : m_listener[name])
        func();

    logger::debug("[ EventManager ] Calling function: %s", name.c_str());
}


