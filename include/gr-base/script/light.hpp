#pragma once

#include "gr-base/ecs/default.hpp"

#include <gr-math/gmath.hpp>

struct Light
{
    enum LightType : int {
        eDirection = 0,
        ePoint     = 1,
        eSpot      = 2,
    };

    LightType type = LightType::eDirection;

    Color color = Color::white;

    float attenuationConstant = 1.0f;
    float attenuationLinear = 0.0f;
    float attenuationQuadratic = 0.0f;

    float angleInnerCone = 0.0f;
    float angleOuterCone = 0.0f;
};

template <>
struct type_traits<Light>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(Light);
};


