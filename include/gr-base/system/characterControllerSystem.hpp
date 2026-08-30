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

namespace engine
{
template <>
struct category_traits<CharacterControllerSystem>
{
    static constexpr TypeCategory value =
        TypeCategory::System;
};
}

