#pragma once

#include "gr-base/core/system.hpp"
#include "gr-base/script/characterController.hpp"

class CharacterInputSystem : public ISystemImpl<
                                Read<CharacterInput>,
                                Write<CharacterController>
                             >
{
public:
private:
    void Update() override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<CharacterInputSystem> =
    TypeCategory::System;


