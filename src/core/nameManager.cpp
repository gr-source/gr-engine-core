#include "gr-base/core/nameManager.hpp"

static const char delimiter = '.';

std::string NameManager::GenerateNameID(std::string name)
{
    auto end = FindDelimiter(delimiter, name);

    if (end != std::string::npos)
    {
        name = name.substr(0, end - 1);
    }

    std::lock_guard<std::mutex> lock(mutex);
    auto it = m_nameID.find(name);
    if (it == m_nameID.end())
    {
        m_nameID.emplace(name, 0);
        return name;
    }

    name = name + delimiter + std::to_string(++it->second);
    return name;
}

void NameManager::RemoveNameID(std::string name)
{
    auto end = FindDelimiter(delimiter, name);

    if (end != std::string::npos) {
        name = name.substr(0, end - 1);
    }

    std::lock_guard<std::mutex> lock(mutex);
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

std::size_t NameManager::FindDelimiter(char delimiter, const std::string &name)
{
    std::size_t pos = 0, start = 0;

    while ((start = name.find(delimiter, pos)) != std::string::npos)
    {
        pos = start + 1;
    }
    return pos == 0 ? std::string::npos : pos;
}

void NameManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex);
    m_nameID.clear();
}



