#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

#include "gr-base/externals/handle.h"

struct MeshFilter
{
    MeshHandle mesh = HANDLE_INVALID;
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


