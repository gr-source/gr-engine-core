#pragma once

#include <gr-math/gmath.hpp>

struct Boundbox
{
    union
    {
        struct
        {
            Vector3 min;
            Vector3 max;
        };
        Vector3 data[2];
    };

    Boundbox Encapsulate(const Boundbox &other) noexcept;
};

inline Boundbox Boundbox::Encapsulate(const Boundbox &other) noexcept
{
    return {
        Math::min(min, other.min),
        Math::max(max, other.max)
    };

}


