#pragma once

#include <gr-math/gmath.hpp>

class Vertex2D {
public:
    Vector2 position;
    Vector2 uv;

    Vertex2D(const Vector2& position = Vector2::zero, const Vector2& uv = Vector2::zero);
};

