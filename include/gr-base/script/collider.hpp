#pragma once

#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

#include <cstdint>

#define INVALID_BODY UINT32_MAX

struct PhysicsShape
{
    btCollisionShape* shape =
        nullptr;
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
    uint32_t body = INVALID_BODY;

    float mass = 1.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Rigidbody> =
    TypeCategory::Component;


