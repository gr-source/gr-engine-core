#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

typedef void (*MethodPointer)();

struct TypeInfo
{
    char name[256]{0};
    Hash64 hash = INVALID_HASH64;
    TypeID index = INVALID_TYPE;

    size_t stride = 0;
    size_t alignment = 0;

    MethodPointer constructor = nullptr;
    MethodPointer destructor = nullptr;

    MethodPointer copy = nullptr;
    MethodPointer move = nullptr;
};

struct ComponentInfo : TypeInfo
{
    MethodPointer serialize = nullptr;
    MethodPointer deserialize = nullptr;

    StorageKind storage = StorageKind::Archetype;

    bool globalCpy = false;
    bool serializable = false;
};

struct SystemInfo : TypeInfo
{
    Signature depends_on = 0;
};



