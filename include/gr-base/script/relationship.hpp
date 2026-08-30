#pragma once

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

struct Relationship
{
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
    Entity prevSibling = INVALID_ENTITY;
};

namespace engine
{
template <>
struct category_traits<Relationship>
{
    static constexpr TypeCategory value =
        TypeCategory::Component;
};
}

