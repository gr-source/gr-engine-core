#pragma once

#include <utility>
#include <cstdarg>
#include <string>
#include <mutex>

struct logger_config
{
    std::string path;
};

class logger
{
public:
    static void Initialize(const logger_config& config);

    static void Shutdown();

    template <typename... Args>
    static void debug(const char* fmt, Args&&... args);

    template <typename... Args>
    static void trace(const char* fmt, Args&&... args);

    template <typename... Args>
    static void info(const char* fmt, Args&&... args);

    template <typename... Args>
    static void error(const char* fmt, Args&&... args);

    template <typename... Args>
    static void critical(const char* fmt, Args&&... args);

    template <typename... Args>
    static void warn(const char* fmt, Args&&... args);

private:
    static void log(const char* prefix, const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        s_logger->vlog(prefix, fmt, ap);
        va_end(ap);
    }

    static logger* s_logger;

    std::mutex m_mutex;
    int m_handle;

    logger(const logger_config &config);
    ~logger();

    void vlog(const char* prefix, const char* fmt, va_list ap);
};

template <typename... Args>
inline void logger::debug(const char* fmt, Args&&... args)
{
    log("[ DEBUG ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logger::trace(const char* fmt, Args&&... args)
{
    log("[ trace ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logger::info(const char* fmt, Args&&... args)
{
    log("[ INFO ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logger::error(const char* fmt, Args&&... args)
{
    log("[ ERROR ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logger::critical(const char* fmt, Args&&... args)
{
    log("[ CRITICAL ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void logger::warn(const char* fmt, Args&&... args)
{
    log("[ WARNING ] ", fmt, std::forward<Args>(args)...);
}


