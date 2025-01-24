#pragma once

#include <sstream>
#include <string>
#include <mutex>
#include <list>

#include "core/string.hpp"

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

class Console {
public:
    static void log(Log::LogType type, const std::string &msg);

    static std::list<Log> GetListLogs();

    static bool bChanged;

    static void Shutdown();

    template <typename ...Args>
    static void Message(Log::LogType type, const std::string &format, Args &&...args);
    
    template <typename ...Args>
    static void LogWarning(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogError(const std::string &format, Args &&...args);

    template <typename ...Args>
    static void LogInfo(const std::string &format, Args &&...args);

private:
    static std::list<Log> m_logs;

    static std::mutex m_mutex;

    static int m_maxLog;
};

template <typename... Args>
inline void Console::Message(Log::LogType type, const std::string &format, Args &&...args) {
    if constexpr (sizeof...(Args) > 0) {
        return log(type, String::format(format, std::forward<Args>(args)...));
    }
    return log(type, format);
}

template <typename... Args>
inline void Console::LogWarning(const std::string &format, Args &&...args) {
    return Console::Message(Log::WARNING, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogError(const std::string &format, Args &&...args) {
    return Console::Message(Log::ERROR, format, std::forward<Args>(args)...);
}

template <typename... Args>
inline void Console::LogInfo(const std::string &format, Args &&...args) {
    return Console::Message(Log::INFO, format, std::forward<Args>(args)...);
}
