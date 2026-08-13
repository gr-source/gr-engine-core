#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/script/characterController.hpp"
#include "gr-base/script/transform.hpp"

class CharacterControllerSystem : public ISystemImpl<Read<CharacterController>, Write<TransformLocal>>
{
public:
private:
    void Update() override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CharacterControllerSystem> =
    TypeCategory::System;

