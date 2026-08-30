#pragma once

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

struct Name
{
    char* value = nullptr;
};

namespace engine
{
template <>
struct category_traits<Name>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}


