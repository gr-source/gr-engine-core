#pragma once

#include "gr-base/externals/handle.h"

#include <stdint.h>

#define COMPONENT_TABLE_SIZE 1024
#define COMPONENT_CHUNK_SIZE 512

#define INVALID_COMPONENT_ID (ComponentID)-1

typedef Handle GameObjectHandle;

typedef Handle ComponentHandle;
typedef uint32_t ComponentID;

typedef uint64_t ComponentHash;

class GameObject;
class Behavior;
class Entity;

