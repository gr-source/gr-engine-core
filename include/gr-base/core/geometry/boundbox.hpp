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

    float area() const noexcept;
};

inline Boundbox Boundbox::Encapsulate(const Boundbox &other) noexcept
{
    return {
        Math::min(min, other.min),
        Math::max(max, other.max)
    };
}


inline float Boundbox::area() const noexcept
{
    Vector3 size = max - min;
    return 2.0f * (size.x * size.y + size.y * size.z + size.z * size.x);
}


