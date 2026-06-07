#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>

#include "gr-base/externals/hash.hpp"

#include "gr-base/externals/handle.h"

#define INVALID_ARCHETYPE (ArchetypeID)-1
#define INVALID_COMPONENT (ComponentID)-1
#define INVALID_SIGNATURE (Signature)-1
#define INVALID_ENTITY (Entity)-1
#define INVALID_TYPE (TypeID)-1

typedef Handle Entity;

typedef uint32_t ComponentID;
typedef uint32_t ArchetypeID;
typedef uint32_t EntityID;
typedef uint32_t ChunkID;
typedef uint32_t TypeID;

typedef uint64_t Signature;

class ArchetypeStorage;
class EntityManager;
class World;
class Type;

// template<typename... Components>
// class View;

struct TypeInfo;

template <typename T>
inline Hash64 GetHash64ByType(const char* key = nullptr) noexcept
{
    static Hash64 hash = hash::fnv1_str(key);
    return hash;
}

enum class StorageKind
{
    Archetype,
    Sparse
};

template <typename T>
struct type_traits
{
    static constexpr StorageKind storage = StorageKind::Archetype;
};


