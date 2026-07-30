#pragma once

#include <string>

#include "gr-base/core/ievent.hpp"
#include "gr-base/core/string.hpp"

struct Console_onAddMessage : IEvent
{
    std::string message;
};

class Console
{
public:
    template <typename ...Args>
    static void LogWarning(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogError(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogInfo(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogDefault(const std::string &format, Args &&...args);

private:
    static void SendMessage(const std::string &msg);
};

template <typename... Args>
inline void Console::LogWarning(const std::string &format, Args &&...args)
{
    SendMessage("{FFFF00}" + String::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void Console::LogError(const std::string &format, Args &&...args)
{
    SendMessage("{FF0000}" + String::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void Console::LogInfo(const std::string &format, Args &&...args)
{
    SendMessage("{00FF00}" + String::format(format, std::forward<Args>(args)...));
}

template <typename... Args>
inline void Console::LogDefault(const std::string &format, Args &&...args)
{
    SendMessage(String::format(format, std::forward<Args>(args)...));
}



