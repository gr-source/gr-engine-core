#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

class CameraSystem : public ISystem
{
public:
    void Update() override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CameraSystem> =
    TypeCategory::System;


