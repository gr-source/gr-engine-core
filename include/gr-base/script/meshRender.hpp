#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include <cstdint>
#include <gr-math/aabb.hpp>

struct MeshRender
{
    MaterialID* materials = nullptr;
    size_t numMaterials = 0;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MeshRender> =
    TypeCategory::Component;

struct RenderBounds
{
    AABB localBounds;
    uint32_t id = UINT32_MAX;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<RenderBounds> =
    TypeCategory::Component;


