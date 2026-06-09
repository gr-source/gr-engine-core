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
struct type_traits<MeshFilter>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(MeshFilter);
};

struct MeshRef
{
    AssetHash hash = INVALID_HASH64;
};

template <>
struct type_traits<MeshRef>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(MeshRef);
};


