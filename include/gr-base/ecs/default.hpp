#pragma once

#include <stdint.h>

#define COMPONENT_TABLE_SIZE 1024
#define COMPONENT_CHUNK_SIZE 512

#define INVALID_COMPONENT_HANDLE (ComponentHandle)-1
#define INVALID_COMPONENT_ID (ComponentID)-1

typedef uint64_t ComponentHandle;
typedef uint32_t ComponentID;

typedef uint64_t EntityID;

class Behavior;
class Entity;


