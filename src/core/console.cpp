#include "gr-base/core/console.hpp"

std::list<Log> Console::m_logs;

bool Console::bChanged = false;

int Console::m_maxLog = 500;

std::mutex Console::m_mutex;

void Console::log(Log::LogType type, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.push_back({type, msg});

    if (m_logs.size() >= m_maxLog)
    {
        m_logs.pop_front();
    }
    Console::bChanged = true;
}

std::list<Log> Console::GetListLogs()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logs;
}

void Console::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    bChanged = false;

    m_logs.clear();
}




