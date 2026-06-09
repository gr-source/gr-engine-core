#pragma once

#include "gr-base/ecs/default.hpp"

struct Name
{
    char* value = nullptr;
};

template <>
struct type_traits<Name>
{
    static constexpr TypeCategory category = TypeCategory::Component;

    static constexpr StorageKind storage = StorageKind::Archetype;

    static constexpr size_t size = sizeof(Name);
};


