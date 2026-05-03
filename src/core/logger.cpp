#include "gr-base/core/logger.hpp"
#include "gr-base/core/file/file.hpp"

#include <sys/types.h>
#include <sys/time.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

// private
logger& logger::GetInstance()
{
    static logger s_logger;
    return s_logger;
}

logger::logger()
{
    std::string dir = gfile::get_config_directory() + "/logs";
    if (!gfile::exists(dir))
        gfile::create_directory(dir);

    std::string file = dir + "/" + __DATE__ + ".txt";

    m_handle = open(file.c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (m_handle == -1)
        perror("open failed");
}

logger::~logger()
{
    if (m_handle != -1)
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

    snprintf(final, sizeof(final),
             "[%02d:%02d:%02d.%03ld] %-*s %s\n",
             tm_info.tm_hour, tm_info.tm_min, tm_info.tm_sec,
             tv.tv_usec / 1000,
             10, prefix, formatted);

    size_t len = strlen(final);

    if (write(m_handle, final, len) != static_cast<ssize_t>(len))
        write(STDERR_FILENO, "Log write failed!\n", 18);
}



