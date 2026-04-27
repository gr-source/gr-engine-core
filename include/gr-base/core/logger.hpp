#pragma once

#include <cstdarg>
#include <mutex>
#include <utility>

class logger
{
public:
    template <typename... Args>
    static void debug(const char* fmt, Args&&... args);

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
        GetInstance().vlog(prefix, fmt, ap);
        va_end(ap);
    }

    static logger& GetInstance();

    std::mutex m_mutex;
    int m_handle;

    logger();
    ~logger();

    void vlog(const char* prefix, const char* fmt, va_list ap);
};

template <typename... Args>
void logger::debug(const char* fmt, Args&&... args)
{
    log("[ DEBUG ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void logger::info(const char* fmt, Args&&... args)
{
    log("[ INFO ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void logger::error(const char* fmt, Args&&... args)
{
    log("[ ERROR ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void logger::critical(const char* fmt, Args&&... args)
{
    log("[ CRITICAL ] ", fmt, std::forward<Args>(args)...);
}

template <typename... Args>
void logger::warn(const char* fmt, Args&&... args)
{
    log("[ WARNING ] ", fmt, std::forward<Args>(args)...);
}


