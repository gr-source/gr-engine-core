#pragma once

#include <string>

enum Scheme
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

