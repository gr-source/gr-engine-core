#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/default.hpp"

class CameraSystem : public System
{
public:
    void Update(Scene* scene) override;
};


