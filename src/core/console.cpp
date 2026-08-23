#include "gr-base/core/console.hpp"
#include "gr-base/managers/eventManager.hpp"

// private
void Console::SendMessage(const std::string &msg)
{
    EventManager::Emit(Console_onAddMessage{
            .message = msg
        });
}



