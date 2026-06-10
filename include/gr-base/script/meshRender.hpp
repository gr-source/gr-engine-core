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




