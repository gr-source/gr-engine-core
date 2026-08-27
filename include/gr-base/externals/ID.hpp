#pragma once

#include <cstdint>
#include <functional>
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

    constexpr explicit operator bool() const
    {
        return value != InvalidValue;
    }

    constexpr operator IdType() const noexcept
    {
        return value;
    }

    constexpr bool operator ==(const ID& other) const noexcept
    {
        return value == other.value;
    }

    constexpr bool operator !=(const ID& other) const noexcept
    {
        return value != other.value;
    }
};

namespace std
{
    template <typename Tag, typename T>
    struct hash<ID<Tag, T>>
    {
        size_t operator()(ID<Tag, T> id) const noexcept
        {
            return std::hash<T>{}(id.value);
        }
    };
}

