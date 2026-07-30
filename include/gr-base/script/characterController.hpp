#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

class btKinematicCharacterController;
class btPairCachingGhostObject;
class btConvexShape;

struct CharacterController
{
    btPairCachingGhostObject* ghostObject = nullptr;
    btKinematicCharacterController* character = nullptr;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CharacterController> =
    TypeCategory::Component;

struct CharacterInput
{
    Vector3 move = type_traits<Vector3>::zero;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CharacterInput> =
    TypeCategory::Component;




