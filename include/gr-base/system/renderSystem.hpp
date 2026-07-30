#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include <vector>

class RenderSystem : public ISystem
{
public:
    RenderSystem() = default;
    virtual ~RenderSystem() override = default;

private:
    std::vector<Entity> m_entities;

    void Update(Scene* scene) override;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<RenderSystem> =
    TypeCategory::System;

