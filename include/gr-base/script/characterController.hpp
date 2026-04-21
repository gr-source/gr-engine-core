#pragma once

#include "gr-base/ecs/component.hpp"

#include "gr-base/physics/default.hpp"

#include <gr-math/gmath.hpp>

class CharacterController : public ComponentImpl<CharacterController>
{
public:
    CharacterController();
    ~CharacterController() override;

    void Start() override;

    void Update() override;

    void SetWalkingDirection(const Vector3& direction);

    bool CanJump();

    void Jump();

private:
    btConvexShape* m_capsule;

    btPairCachingGhostObject* m_ghost;

    btKinematicCharacterController* m_character;
};

REGISTER_COMPONENT(CharacterController, (uint64_t)-1)

