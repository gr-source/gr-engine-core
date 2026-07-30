#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

#include <cstdint>

#define INVALID32 UINT32_MAX

struct PhysicsShape
{
    uint32_t shapeID = INVALID32;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<PhysicsShape> =
    TypeCategory::Component;

struct BoxCollider
{
};

template <>
inline constexpr TypeCategory engine::type_traits::category<BoxCollider> =
    TypeCategory::Component;

struct Rigidbody
{
    uint32_t body = INVALID32;
    uint32_t motion = INVALID32;

    float mass = 1.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Rigidbody> =
    TypeCategory::Component;


