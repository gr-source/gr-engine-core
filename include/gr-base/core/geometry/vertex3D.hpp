#pragma once

#include <gr-math/gmath.hpp>

struct GPUVec2
{
    float x, y;

    GPUVec2() = default;

    GPUVec2(const Vector2& v) : x(v.x), y(v.y)
    {}

    GPUVec2& operator =(const Vector2& v)
    {
        x = v.x;
        y = v.y;

        return *this;
    }
};


struct GPUVec4
{
    float x, y, z, w;

    GPUVec4(const Vector3& v) : x(v.x), y(v.y), z(v.z), w(0.0f)
    {}

    GPUVec4() = default;

    GPUVec4& operator =(const Vector3& v)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = 0.0f;

        return *this;
    }
};

struct Vertex3D
{
    GPUVec4 position;
    GPUVec4 normal;
    GPUVec2 texCoords;
};

static_assert(sizeof(Vertex3D) == 40, "www");


