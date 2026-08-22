#include "gr-base/core/console.hpp"
#include "gr-base/managers/eventManager.hpp"

// private
void Console::SendMessage(const std::string &msg)
{
    EventManager::EmitAsync(Console_onAddMessage{
            .message = msg
        });
}



