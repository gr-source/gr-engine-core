#include "gr-base/core/logger.hpp"

#include "gr-base/core/file/file.hpp"

#include <sys/types.h>
#include <sys/time.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

logger* logger::s_logger = nullptr;

void logger::Initialize(const logger_config* config)
{
    if (s_logger == nullptr)
        s_logger = new logger(config);
}

void logger::Shutdown()
{
    if (s_logger != nullptr)
        delete s_logger;
    s_logger = nullptr;
}

// private
logger::logger(const logger_config* config) : m_handle(STDOUT_FILENO)
{
    if (config != nullptr)
    {
        if (!gfile::exists(config->path))
            gfile::create_directory(config->path);

        std::string file =
            config->path + "/" + getfilename();

        m_handle = open(file.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
        if (m_handle == -1)
            perror("open failed");
    }
}

logger::~logger()
{
    if (m_handle != -1 && m_handle != STDOUT_FILENO)
    {
        const char* newline = "\n";
        write(m_handle, newline, 1);

        close(m_handle);
    }
}

void logger::vlog(const char* prefix, const char* fmt, va_list ap)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    char formatted[512];
    vsnprintf(formatted, sizeof(formatted), fmt, ap);

    char final[1024];
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    time_t rawtime = tv.tv_sec;
    struct tm tm_info;
    localtime_r(&rawtime, &tm_info);

    size_t len =
        snprintf(final, sizeof(final),
             "[%02d:%02d:%02d.%03ld] %-*s %s\n",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec,
             tv.tv_usec / 1000,
             10, prefix, formatted);

    if (len > 0)
    {
        if (write(m_handle, final, len) != len)
            write(STDERR_FILENO, "Log write failed!\n", 18);
    }
}

std::string logger::getfilename() const
{
    char filename[32];

    time_t now = time(nullptr);
    tm tm{};
    localtime_r(&now, &tm);

    strftime(filename, sizeof(filename), "%Y-%m-%d.txt", &tm);

    return filename;
}


