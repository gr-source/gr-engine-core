#pragma once

#include "gr-base/core/file/default.hpp"

#include "gr-base/ecs/default.hpp"

#include <type_traits>

typedef void (*MethodPointer)();

typedef struct TypeInfo
{
    const char* name;
    Hash64 hash;
    TypeID index;
    size_t stride;
    size_t alignment;

    bool globalCpy;
    bool serializable;

    StorageKind storage;

    MethodPointer constructor;
    MethodPointer destructor;
    MethodPointer copy;
    MethodPointer move;

    MethodPointer serialize;
    MethodPointer deserialize;
} TypeInfo;

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


