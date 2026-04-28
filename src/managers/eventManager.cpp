#include "gr-base/managers/eventManager.hpp"
#include "gr-base/core/logger.hpp"

void EventManager::Notify(const std::string &name)
{
    GetInstance().notify(name);
}

// private
EventManager& EventManager::GetInstance()
{
    static EventManager s_EventManager;
    return s_EventManager;
}

EventManager::EventManager()
{
    logger::info("[EventManager] Initialized.");
}

EventManager::~EventManager()
{
    logger::info("[EventManager] Shutdown.");
}

void EventManager::attach(const std::string &name, std::function<void ()> func)
{
    m_listener[name].push_back(func);

    logger::debug("[EventManager] Attached function: %s", name.c_str());
}

void EventManager::notify(const std::string &name)
{
    for (auto& func : m_listener[name])
        func();

    logger::debug("[EventManager] Calling function: %s", name.c_str());
}


