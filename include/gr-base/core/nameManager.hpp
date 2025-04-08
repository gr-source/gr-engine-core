#pragma once

#include <unordered_map>
#include <string>
#include <mutex>

class NameManager {
public:
    std::string GenerateNameID(std::string name);

    void RemoveNameID(std::string name);

    void clear();

private:
    std::unordered_map<std::string, int> m_nameID;

    static std::size_t FindDelimiter(char delimiter, const std::string &name);

protected:
    static std::mutex mutex;
};


