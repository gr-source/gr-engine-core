#pragma once

#include <unordered_map>
#include <string>

class NameManager {
public:
    static void Shutdown();

    static std::string GenerateNameID(std::string name);

    static void RemoveNameID(std::string name);

private:
    static std::unordered_map<std::string, int> m_nameID;

    static std::size_t FindDelimiter(char delimiter, const std::string &name);
};


