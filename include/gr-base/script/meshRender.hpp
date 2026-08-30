#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include <cstdint>
#include <gr-math/aabb.hpp>

struct MeshRender
{
    MaterialRef* materials = nullptr;
    size_t numMaterials = 0;
};

namespace engine
{
template <>
struct category_traits<MeshRender>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

struct RenderBounds
{
    AABB localBounds;
    uint32_t id = UINT32_MAX;
};

namespace engine
{
template <>
struct category_traits<RenderBounds>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}


