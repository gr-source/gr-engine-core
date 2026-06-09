#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/default.hpp"

class CameraSystem : public ISystem
{
public:
    void Update(Scene* scene) override;
};

template <>
struct type_traits<CameraSystem>
{
    static constexpr TypeCategory category = TypeCategory::System;

    static constexpr size_t size = sizeof(CameraSystem);
};


