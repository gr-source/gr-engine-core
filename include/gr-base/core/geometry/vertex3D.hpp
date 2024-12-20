#pragma once

#include <gr-math/gmath.hpp>

struct Vertex3D {
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int mIdx;

    Vertex3D(const Vector3 &position = Vector3::zero, const Vector3 &normal = Vector3::zero, const Vector2 &texCoord = Vector2::zero, int mIdx = 0);
};
