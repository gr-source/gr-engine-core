#pragma once

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/default.hpp"

class TransformSystem : public ISystem
{
public:
    void Update(Scene* scene) override;
};

template <>
struct type_traits<TransformSystem>
{
    static constexpr TypeCategory category = TypeCategory::System;
};


