#pragma once

#include <cstddef>

using BlockSize = std::size_t;

template <typename T, BlockSize S>
struct DataBlock
{
    struct BlockEntry
    {
        T &data;
        bool &isActive;

        inline BlockEntry &operator=(bool active)
        {
            isActive = active;
            return *this;
        }

        inline BlockEntry &operator=(const T &value)
        {
            data = value;
            return *this;
        }
    };

    T data[S];
    bool isActive[S];

    inline BlockEntry operator[](BlockSize i)
    {
        return {data[i], isActive[i]};
    }
};



