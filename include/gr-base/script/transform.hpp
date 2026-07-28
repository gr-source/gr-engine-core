#pragma once

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include <gr-math/gmath.hpp>

struct TransformLocal
{
    Vector3 position = type_traits<Vector3>::zero;
    Quaternion rotation = type_traits<Quaternion>::identity;
    Vector3 scale = type_traits<Vector3>::one;

    bool dirty = true;
};

struct TransformWorld
{
    Matrix4x4 localMatrix = type_traits<Matrix4x4>::identity;

    Matrix4x4 worldMatrix = type_traits<Matrix4x4>::identity;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<TransformLocal> =
    TypeCategory::Component;

template <>
inline constexpr TypeCategory engine::type_traits::category<TransformWorld> =
    TypeCategory::Component;


