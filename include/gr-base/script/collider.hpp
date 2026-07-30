#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

#include <cstdint>

#define INVALID32 UINT32_MAX

class btCollisionShape;
class btMotionState;
class btRigidBody;

struct PhysicsShape
{
    btCollisionShape* shape = nullptr;
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

struct CapsuleCollider
{
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CapsuleCollider> =
    TypeCategory::Component;

struct Rigidbody
{
    btMotionState* motionState = nullptr;
    btRigidBody* body = nullptr;

    float mass = 1.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Rigidbody> =
    TypeCategory::Component;


