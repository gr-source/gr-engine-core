#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/asset/default.hpp"

#include "gr-base/core/default.hpp"
#include <gr-math/aabb.hpp>

struct MeshRender
{
    MaterialHandle* materials = nullptr;
    size_t numMaterials = 0;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MeshRender> =
    TypeCategory::Component;

struct MaterialRef
{
    AssetHash* materials = nullptr;
    size_t numMaterials = 0;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MaterialRef> =
    TypeCategory::Component;


struct RenderCache
{
    uint32_t version = UINT32_MAX;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<RenderCache> =
    TypeCategory::Component;


struct RenderBounds
{
    AABB local;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<RenderBounds> =
    TypeCategory::Component;


