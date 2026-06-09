#pragma once

#include "gr-base/managers/default.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/ecs/default.hpp"

struct MeshRender
{
    MaterialHandle* materials = nullptr;
    size_t numMaterials = 0;
};

template <>
struct type_traits<MeshRender>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(MeshRender);
};

struct MaterialRef
{
    AssetHash* materials = nullptr;
    size_t numMaterials = 0;
};

template <>
struct type_traits<MaterialRef>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(MaterialRef);
};




