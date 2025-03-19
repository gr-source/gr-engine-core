#pragma once

#include <cstddef>
#include <string>
#include <map>

class Scriptpriority {
public:
    static void add(const std::string &name);

    static void sub(const std::string &name);

    static void rm(const std::string &name);

    static std::size_t get(const std::string &name);

private:
    static std::map<std::string, std::size_t> map;
};

