#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/default.hpp"

class CameraSystem : public ISystem
{
public:
    void Update(Scene* scene) override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraSystem> =
    TypeCategory::System;


