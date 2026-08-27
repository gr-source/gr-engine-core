#pragma once

#include "gr-base/ecs/type_traits.hpp"

#include "gr-base/core/default.hpp"

#include "gr-base/asset/default.hpp"

struct MeshFilter
{
    MeshID mesh =
        MeshID::Invalid();
};

template <>
inline constexpr TypeCategory engine::type_traits::category<MeshFilter> =
    TypeCategory::Component;


