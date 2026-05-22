#pragma once

#include <cassert>
#include <cstdint>
#include <cstddef>

#include "gr-base/externals/hash.hpp"

#include "gr-base/externals/handle.h"

#define INVALID_ARCHETYPE (ArcheTypeID)-1
#define INVALID_COMPONENT (ComponentID)-1
#define INVALID_SIGNATURE (Signature)-1
#define INVALID_HASH64 (Hash64)-1
#define INVALID_ENTITY (Entity)-1
#define INVALID_TYPE (TypeID)-1

typedef Handle Entity;

typedef uint32_t ComponentID;
typedef uint32_t ArcheTypeID;
typedef uint32_t EntityID;
typedef uint32_t ChunkID;
typedef uint32_t TypeID;

typedef uint64_t Signature;

class ArchetypeStorage;
class EntityManager;
class World;
class Type;

template<typename... Components>
class View;

struct TypeInfo;

template <typename T>
inline Hash64 GetHash64ByType(const char* key = nullptr) noexcept
{
    static Hash64 hash = hash::fnv1_str(key);
    return hash;
}

