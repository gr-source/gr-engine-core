#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>

#include "gr-base/externals/hash.hpp"

#include "gr-base/externals/handle.h"

#include <stdint.h>

#define INVALID_ARCHETYPE UINT32_MAX
#define INVALID_COMPONENT UINT32_MAX
#define INVALID_CHUNK UINT32_MAX
#define INVALID_TYPE UINT32_MAX

#define INVALID_SIGNATURE UINT64_MAX
#define INVALID_ENTITY UINT64_MAX

typedef Handle Entity;

typedef uint32_t ComponentID;
typedef uint32_t ArchetypeID;
typedef uint32_t EntityID;
typedef uint32_t ChunkID;
typedef uint32_t TypeID;

typedef uint64_t Signature;

class ArchetypeStorage;
class EntityManager;
class TypeRegistry;
class World;

struct TypeInfo2;

struct ComponentInfo;
struct SystemInfo;

enum class TypeCategory
{
    Unknown,
    Component,
    System
};

enum class StorageKind
{
    Archetype,
    Sparse
};

template <typename T>
inline constexpr std::string_view get_type_name()
{
#if defined(__clang__)
    constexpr auto prefix = std::string_view{"[T = "};
    constexpr auto suffix = std::string_view{"]"};
    constexpr auto func = std::string_view{__PRETTY_FUNCTION__};
#else // GCC
    constexpr auto prefix = std::string_view{"with T = "};
    constexpr auto suffix = std::string_view{";"};
    constexpr auto func = std::string_view{__PRETTY_FUNCTION__};
#endif
    constexpr auto start = func.find(prefix) + prefix.size();
    constexpr auto end = func.find(suffix);
    return func.substr(start, end - start);
}

namespace engine::type_traits
{
    template <typename T>
    inline constexpr std::string_view name =
        get_type_name<T>();

    template <typename T>
    inline constexpr Hash64 hash =
        engine::hash::fnv1_str(engine::type_traits::name<T>);

    template <typename T>
    inline constexpr TypeCategory category =
        TypeCategory::Unknown;

    template <typename T>
    inline constexpr StorageKind storage =
        StorageKind::Archetype;

    template <typename T>
    inline constexpr size_t size =
        sizeof(T);

    template <typename T>
    inline constexpr size_t alignment =
        alignof(T);
}


