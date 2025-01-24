#include "core/nameManager.hpp"

static const char delimiter = '&';

std::unordered_map<std::string, int> NameManager::m_nameID;

void NameManager::Shutdown() {
    m_nameID.clear();
}

std::string NameManager::GenerateNameID(std::string name) {
    auto lastPosition = FindDelimiter(delimiter, name);

    if (lastPosition != std::string::npos) {
        name = name.substr(0, lastPosition - 1);
    }

    auto it = m_nameID.find(name);
    if (it == m_nameID.end()) {
        m_nameID.emplace(name, 0);
        return name;
    }

    name = name + delimiter + std::to_string(++it->second);
    return name;
}

void NameManager::RemoveNameID(std::string name) {
    auto lastPosition = FindDelimiter(delimiter, name);

    if (lastPosition != std::string::npos) {
        name = name.substr(0, lastPosition - 1);
    }

    auto it = m_nameID.find(name);
    if (it == m_nameID.end()) {
        return;
    }
    if (it->second <= 0) {
        m_nameID.erase(it);
        return;
    }
    it->second--;
}

std::size_t NameManager::FindDelimiter(char delimiter, const std::string &name) {
    std::size_t lastPosition = 0;

    while (true) {
        auto pos = name.find(delimiter, lastPosition);
        if (pos == std::string::npos) {
            break;
        }

        lastPosition = pos + 1;
    }
    return lastPosition == 0 ? std::string::npos : lastPosition;
}
