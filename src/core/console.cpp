#include "gr-base/core/console.hpp"

// public
std::list<Log> Console::GetLogs()
{
    return GetInstance().getLogs();
}

bool Console::IsChanged()
{
    return GetInstance().isChanged();
}

void Console::StopChange()
{
    return GetInstance().stopChange();
}

// private
Console &Console::GetInstance()
{
    static Console instance;
    return instance;
}

Console::Console() : m_maxLog(500), m_changed(false)
{}

void Console::log(Log::LogType type, const std::string &msg)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logs.push_back({type, msg});

    if (m_logs.size() >= m_maxLog)
    {
        m_logs.pop_front();
    }
    m_changed = true;
}

const std::list<Log> &Console::getLogs()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logs;
}

bool Console::isChanged()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_changed;
}

void Console::stopChange()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_changed = false;
}


