#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

struct MeshFilter
{
    MeshID mesh =
        INVALID_ID32;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MeshFilter> =
    TypeCategory::Component;

struct MeshRef
{
    AssetHash hash = INVALID_HASH64;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MeshRef> =
    TypeCategory::Component;


