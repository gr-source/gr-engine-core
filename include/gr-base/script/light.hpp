#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include <gr-render/color.hpp>

#include <gr-math/gmath.hpp>

typedef uint32_t LightType;

enum LightType_
{
    LightType_Direction,
    LightType_Point,
    LightType_Spot,
    LightType_Count
};

struct Light
{
    LightType type = LightType_Direction;

    Color color = Color::White();

    float attenuationConstant = 1.0f;
    float attenuationLinear = 0.70f;
    float attenuationQuadratic = 1.80f;

    float angleInnerCone = 0.0f;
    float angleOuterCone = 0.0f;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Light> =
    TypeCategory::Component;


