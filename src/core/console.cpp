#include "gr-base/core/console.hpp"
#include "gr-base/managers/eventManager.hpp"

// private
void Console::SendMessage(const std::string &msg)
{
    Console_onAddMessage event;
    event.message = msg;

    EventManager::Notify(&event);
}



