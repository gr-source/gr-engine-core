#pragma once

#include <string>

enum class Scheme
{
    Engine,
    Project,
    Mod,
    Count
};

struct URI
{
    Scheme scheme;
    std::string path;

    std::string toString() const;

    static URI parse(const std::string& uri);
};

