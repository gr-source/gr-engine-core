#include "gr-base/managers/eventManager.hpp"

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

void EventManager::attach(const std::string &name, std::function<void ()> func)
{
    m_listener[name].push_back(func);
}

void EventManager::notify(const std::string &name)
{
    for (auto& func : m_listener[name])
        func();
}


