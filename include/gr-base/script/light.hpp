#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-math/gmath.hpp>

enum class LightType : int
{
    eDirection,
    ePoint,
    eSpot
};

struct Light
{
    LightType type = LightType::eDirection;

    Color color = COLOR_WHITE;

    float attenuationConstant = 1.0f;
    float attenuationLinear = 0.70f;
    float attenuationQuadratic = 1.80f;

    float angleInnerCone = 0.0f;
    float angleOuterCone = 0.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Light> =
    TypeCategory::Component;


