#pragma once

#include "gr-base/ecs/default.hpp"

struct Relationship
{
    Entity parent = INVALID_ENTITY;
    Entity firstChild = INVALID_ENTITY;
    Entity nextSibling = INVALID_ENTITY;
    Entity prevSibling = INVALID_ENTITY;
};

template <>
struct type_traits<Relationship>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(Relationship);
};

