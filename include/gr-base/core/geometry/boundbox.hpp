#pragma once

#include <gr-math/gmath.hpp>

class Boundbox {
public:
    Vector3 min;
    Vector3 max;

    Boundbox(const Vector3& min = Vector3::zero, const Vector3& max = Vector3::zero);
};

