#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

struct MeshFilter
{
    MeshRef mesh;
};

namespace engine
{
template <>
struct category_traits<MeshFilter>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}


