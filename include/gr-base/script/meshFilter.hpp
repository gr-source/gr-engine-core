#pragma once

#include "gr-base/ecs/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/externals/handle.h"

#include "gr-base/managers/default.hpp"

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


