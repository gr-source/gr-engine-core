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

namespace engine
{
template <>
struct category_traits<CharacterInputSystem>
{
    static constexpr TypeCategory value =
        TypeCategory::System;
};
}


