#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <type_traits>

typedef void (*MethodPointer)();

struct TypeInfo2
{
    const char* name = nullptr;
    Hash64 hash = INVALID_HASH64;
    TypeID index = INVALID_TYPE;

    size_t stride = 0;
    size_t alignment = 0;

    MethodPointer constructor = nullptr;
    MethodPointer destructor = nullptr;

    MethodPointer copy = nullptr;
    MethodPointer move = nullptr;
};

struct ComponentInfo : TypeInfo2
{
    MethodPointer serialize = nullptr;
    MethodPointer deserialize = nullptr;

    StorageKind storage = StorageKind::Archetype;

    bool globalCpy = false;
    bool serializable = false;
};

struct SystemInfo : TypeInfo2
{
    Signature depends_on = 0;
};

template<typename, typename = void>
struct HasSerialize : std::false_type {};

template<typename T>
struct HasSerialize<
    T,
    std::void_t<
        decltype(
            T::Serialize(
                std::declval<Stream*>(),
                std::declval<const T*>()
            )
        )
    >
> : std::true_type {};

template<typename, typename = void>
struct HasDeserialize : std::false_type {};

template<typename T>
struct HasDeserialize<
    T,
    std::void_t<
        decltype(
            T::Deserialize(
                std::declval<Stream*>(),
                std::declval<T*>()
            )
        )
    >
> : std::true_type {};


