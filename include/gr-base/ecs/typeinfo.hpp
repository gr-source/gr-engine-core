#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/signature.hpp"
#include "gr-base/ecs/type_traits.hpp"
#include "gr-base/ecs/default.hpp"

using ConstructorFn = void(*)(void*, const void*);
using DestructorFn  = void(*)(void*);
using CopyFn        = void(*)(void*, const void*);
using MoveFn        = void(*)(void*, void*);

typedef void (*MethodPointer)();

struct TypeInfo
{
    char name[256]{0};
    Hash64 hash = INVALID_HASH64;
    TypeID index = INVALID_TYPE;

    size_t stride = 0;
    size_t alignment = 0;

    ConstructorFn constructor;
    DestructorFn destructor;

    CopyFn copy;
    MoveFn move;
};

struct ComponentInfo : TypeInfo
{
    MethodPointer serialize = nullptr;
    MethodPointer deserialize = nullptr;

    StorageKind storage = StorageKind::Archetype;

    bool isTriviallyCopyable = false;
    bool serializable = false;
};

struct SystemInfo : TypeInfo
{
    Signature depends_on = 0;
};



