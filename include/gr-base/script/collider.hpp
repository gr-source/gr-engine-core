#pragma once

#include "gr-base/ecs/component.hpp"

#include "gr-base/physics/default.hpp"

#include <memory>

class Collider : public ComponentImpl<Collider>
{
public:
    std::shared_ptr<btCollisionShape> shape;

    Collider();

    virtual void Init() override {}
};

// REGISTER_COMPONENT(Collider, (uint64_t)-1)

