#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

struct TransformLocal
{
    Vector3 position = type_traits<Vector3>::zero;
    Quaternion rotation = type_traits<Quaternion>::identity;
    Vector3 scale = type_traits<Vector3>::one;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<TransformLocal> =
    TypeCategory::Component;

struct TransformWorld
{
    Matrix4x4 local = type_traits<Matrix4x4>::identity;

    Matrix4x4 world = type_traits<Matrix4x4>::identity;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<TransformWorld> =
    TypeCategory::Component;


struct Static
{
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Static> =
    TypeCategory::Component;

