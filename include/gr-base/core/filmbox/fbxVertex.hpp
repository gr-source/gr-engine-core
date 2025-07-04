#pragma once

#include <gr-math/gmath.hpp>

class FbxVertex {
public:
    Vector3 position;
    Vector3 normal;
    Vector2 texCoord;
    int index;

    float weight[4];
    int boneIndex[4];

    FbxVertex(const Vector3 &position = Vector3::zero, const Vector3 &normal = Vector3::zero, const Vector2 &texCoord = Vector2::zero, int index = 0);
};


