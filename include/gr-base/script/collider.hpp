#pragma once

#include "gr-base/ecs/behavior.hpp"

#include "gr-base/physics/default.hpp"

#include <memory>

class Collider : public BehaviorImpl<Collider>
{
public:
    std::shared_ptr<btCollisionShape> shape;

    Collider();

    virtual void Init() override {}
};

REGISTER_COMPONENT(Collider, (uint64_t)-1)

