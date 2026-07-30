#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"

class CharacterControllerSystem : public ISystem
{
public:
private:
    void Update() override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CharacterControllerSystem> =
    TypeCategory::System;

