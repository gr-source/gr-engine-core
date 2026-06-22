#pragma once

#include <gr-math/gmath.hpp>

struct Vertex3D
{
    Vector3 position;
    Vector3 normal;
    Vector2 texCoords;
};

// static_assert(sizeof(Vertex3D) == 40, "www");


