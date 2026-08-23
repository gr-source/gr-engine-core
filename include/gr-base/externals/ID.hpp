#pragma once

#include <cstdint>
#include <limits>

#include <cstdint>
#include <limits>

template <typename Tag, typename IdType = uint32_t>
struct ID
{
    static constexpr IdType InvalidValue = std::numeric_limits<IdType>::max();

    static constexpr ID Invalid()
    {
        return ID{InvalidValue};
    }

    IdType value = InvalidValue;

    explicit operator bool() const
    {
        return value != InvalidValue;
    }
};

