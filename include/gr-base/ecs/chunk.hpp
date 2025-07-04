#pragma once

#include <cstddef>
#include <cstdlib>

template <typename T, std::size_t C>
class Chunk
{
public:
    T data[C];
    bool usage[C];

    std::size_t activecount;
};

