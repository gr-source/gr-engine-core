#include "gr-base/core/nameManager.hpp"

#define DELIMITER '.'

std::string NameManager::GenerateNameID(std::string name)
{
    auto end = name.rfind(DELIMITER);

    if (end != std::string::npos)
    {
        name = name.substr(0, end);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_nameID.find(name);
    if (it == m_nameID.end())
    {
        m_nameID.emplace(name, 0);
        return name;
    }

    name = name + DELIMITER + std::to_string(++it->second);
    return name;
}

void NameManager::RemoveNameID(std::string name)
{
    auto end = name.rfind(DELIMITER);

    if (end != std::string::npos)
    {
        name = name.substr(0, end);
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_nameID.find(name);
    if (it == m_nameID.end())
    {
        return;
    }
    
    it->second--;

    if (it->second <= 0)
    {
        m_nameID.erase(it);
        return;
    }
}

void NameManager::clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_nameID.clear();
}



