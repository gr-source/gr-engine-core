#pragma once

#include <gr-math/gmath.hpp>

struct Transform
{
    Vector3 position = Vector3::zero;

    Quaternion rotation = Quaternion::identity;

    Vector3 scale = Vector3::one;

    Matrix4x4 localMatrix = Matrix4x4::identityMatrix;

    Matrix4x4 worldMatrix = Matrix4x4::identityMatrix;

    bool dirty = true;
};




