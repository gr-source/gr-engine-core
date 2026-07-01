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

struct TypeInfo;

struct ComponentInfo;
struct SystemInfo;


namespace engine::ecs::detail
{
    typedef struct location
    {
        uint32_t chunk;
        uint32_t row;
    } location;
}



