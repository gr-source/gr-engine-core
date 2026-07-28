#pragma once

#include <string>
#include <mutex>
#include <list>

#include "gr-base/core/ievent.hpp"
#include "gr-base/core/string.hpp"

struct Log {
    enum LogType {
        DEFAULT = 0,
        ERROR = 1,
        WARNING = 2,
        INFO = 3
    };

    LogType type;
    std::string message;
};

struct Console_onAddMessage : IEvent
{
    Log log;
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

    static std::list<Log> GetLogs();

    static bool IsChanged();

    static void StopChange();

private:
    std::list<Log> m_logs;

    std::mutex m_mutex;

    int m_maxLog;

    bool m_changed;

    static Console &GetInstance();

    Console();

    void log(Log::LogType type, const std::string &msg);

    template <typename ...Args>
    void message(Log::LogType type, const std::string &format, Args &&...args);

    std::list<Log> getLogs();

    bool isChanged();

    void stopChange();
};

template <typename... Args>
inline void Console::message(Log::LogType type, const std::string &format, Args &&...args)
{
    if constexpr (sizeof...(Args) > 0)
        return log(type, String::format(format, std::forward<Args>(args)...));
    return log(type, format);
}

template <typename... Args>
inline void Console::LogWarning(const std::string &format, Args &&...args) {
    return GetInstance().message(Log::WARNING, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogError(const std::string &format, Args &&...args) {
    return GetInstance().message(Log::ERROR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogInfo(const std::string &format, Args &&...args) {
    return GetInstance().message(Log::INFO, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogDefault(const std::string &format, Args &&...args)
{
    return GetInstance().message(Log::DEFAULT, format, std::forward<Args>(args)...);
}



