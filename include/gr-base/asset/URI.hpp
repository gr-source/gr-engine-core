#pragma once

#include <string>

enum Scheme_
{
    Scheme_Engine,
    Scheme_Project,
    Scheme_Mod,
    Scheme_Count
};

struct URI
{
    Scheme_ scheme;
    std::string path;

    std::string toString() const;

    static URI parse(const std::string& uri);
};

