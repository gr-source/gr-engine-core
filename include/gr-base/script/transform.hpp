#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

struct TransformLocal
{
    Vector3 position = type_traits<Vector3>::zero;
    Quaternion rotation = type_traits<Quaternion>::identity;
    Vector3 scale = type_traits<Vector3>::one;
};

namespace engine
{
template <>
struct category_traits<TransformLocal>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct TransformWorld
{
    Matrix4x4 local = type_traits<Matrix4x4>::identity;

    Matrix4x4 world = type_traits<Matrix4x4>::identity;
};

namespace engine
{
template <>
struct category_traits<TransformWorld>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct Static
{
};

namespace engine
{
template <>
struct category_traits<Static>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

