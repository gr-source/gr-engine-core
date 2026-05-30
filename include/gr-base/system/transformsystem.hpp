#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/default.hpp"

class TransformSystem : public System
{
public:
    void Update(Scene* scene) override;
};

