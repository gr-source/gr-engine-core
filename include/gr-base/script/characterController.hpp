#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

class KinematicCharacterController;
class btPairCachingGhostObject;
class btConvexShape;

struct CharacterController
{
    btPairCachingGhostObject* ghostObject = nullptr;
    KinematicCharacterController* character = nullptr;
};

namespace engine
{
template <>
struct category_traits<CharacterController>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct CharacterInput
{
    Vector3 move = type_traits<Vector3>::zero;
    bool jump = false;
};

namespace engine
{
template <>
struct category_traits<CharacterInput>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct CharacterState
{
    bool canJump = false;
};

namespace engine
{
template <>
struct category_traits<CharacterState>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}


