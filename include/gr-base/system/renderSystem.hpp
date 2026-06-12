#pragma once

#include "gr-base/core/default.hpp"

#include "gr-base/core/system.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

#include "gr-base/managers/default.hpp"

#include <gr-render/shader.hpp>

class RenderSystem : public ISystem
{
public:
    void Update(Scene* scene) override;

private:
    void Setuplight(World* world, gr::Shader* shader);
};

template <>
inline constexpr TypeCategory engine::type_traits::category<RenderSystem> =
    TypeCategory::System;

