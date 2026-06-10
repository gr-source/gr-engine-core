#pragma once

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/gmath.hpp>

struct Transform
{
    Vector3 position = Vector3::zero;
    Quaternion rotation = Quaternion::identity;
    Vector3 scale = Vector3::one;

    Matrix4x4 localMatrix = Matrix4x4::identityMatrix;

    Matrix4x4 worldMatrix = Matrix4x4::identityMatrix;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Transform> =
    TypeCategory::Component;



