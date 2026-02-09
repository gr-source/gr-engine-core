#pragma once

#include "gr-base/ecs/behavior.hpp"

#include "gr-base/physics/default.hpp"

#include <gr-math/gmath.hpp>

class CharacterController : public BehaviorImpl<CharacterController>
{
public:
    CharacterController();
    ~CharacterController() override;

    void Start() override;

    void Update() override;

    void SetWalkingDirection(const Vector3& direction);

private:
    btConvexShape* m_capsule;

    btPairCachingGhostObject* m_ghost;

    btKinematicCharacterController* m_character;
};

REGISTER_COMPONENT(CharacterController, (uint64_t)-1)

