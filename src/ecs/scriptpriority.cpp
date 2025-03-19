#include "gr-base/ecs/scriptpriority.hpp"
#include <cstddef>

std::map<std::string, std::size_t> Scriptpriority::map;

void Scriptpriority::add(const std::string &name)
{
    auto it = map.find(name);
    if (it == map.end())
    {
        map.emplace(name, 1);
        return;
    }
    it->second++;
}

void Scriptpriority::sub(const std::string &name)
{
    auto it = map.find(name);
    if (it == map.end())
    {
        return;
    }
    it->second--;

    if (!it->second)
    {
        map.erase(it);
    }
}

void Scriptpriority::rm(const std::string &name)
{
    map.erase(name);
}

std::size_t Scriptpriority::get(const std::string &name)
{
    auto it = map.find(name);
    if (it == map.end())
    {
        return 0;
    }

    return it->second;
}




