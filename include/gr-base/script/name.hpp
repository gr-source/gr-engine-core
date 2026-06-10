#pragma once

#include "gr-base/ecs/default.hpp"

struct Name
{
    char* value = nullptr;
};

template <>
inline constexpr TypeCategory engine::type_traits::category<Name> =
    TypeCategory::Component;


